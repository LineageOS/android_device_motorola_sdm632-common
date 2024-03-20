/*
   Copyright (c) 2016, The CyanogenMod Project
   Copyright (c) 2019, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/sysinfo.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using android::base::SetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
    nullptr,
    "bootimage.",
    "odm.",
    "odm_dlkm.",
    "product.",
    "system.",
    "system_dlkm.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void OverrideMemoryProperties() {
    struct sysinfo sys;
    std::string heapstartsize;
    std::string heapgrowthlimit;
    std::string heapsize;
    std::string heaptargetutilization;
    std::string heapminfree;
    std::string heapmaxfree;

    sysinfo(&sys);

    if (sys.totalram > 5072ull * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram > 3072ull * 1024 * 1024) {
        // from - phone-xxhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        // from - phone-xhdpi-2048-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.75";
        heapminfree = "512k";
        heapmaxfree = "8m";
    }

    OverrideProperty("dalvik.vm.heapstartsize", heapstartsize.c_str());
    OverrideProperty("dalvik.vm.heapgrowthlimit", heapgrowthlimit.c_str());
    OverrideProperty("dalvik.vm.heapsize", heapsize.c_str());
    OverrideProperty("dalvik.vm.heaptargetutilization", heaptargetutilization.c_str());
    OverrideProperty("dalvik.vm.heapminfree", heapminfree.c_str());
    OverrideProperty("dalvik.vm.heapmaxfree", heapmaxfree.c_str());
}

void OverrideCarrierProperties() {
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        OverrideProperty(prop_name.c_str(), value);
    };

    // Setting carrier prop
    std::string carrier = GetProperty("ro.boot.carrier", "unknown");
    OverrideProperty("ro.carrier", carrier.c_str());

    std::string sku = GetProperty("ro.boot.hardware.sku", "");
    if (sku == "XT1952-T") {
        /* T-Mobile REVVLRY */
        for (const auto &source : RO_PROP_SOURCES) {
            ro_prop_override(source, "fingerprint", "motorola/channel_revvl/channel:10/QPY30.85-18/6572f:user/release-keys", true);
            ro_prop_override(source, "model", "REVVLRY", true);
            ro_prop_override(source, "name", "channel_revvl", true);
        }
        ro_prop_override(nullptr, "description", "channel_revvl-user 10 QPY30.85-18 6572f release-keys", false);
        ro_prop_override(nullptr, "product", "channel_revvl", false);
        OverrideProperty("persist.vendor.radio.customer_mbns", "tmo_usa_ims_default.mbn;sprint_usa_ims.mbn");
        OverrideProperty("persist.vendor.radio.data_con_rprt", "1");
        OverrideProperty("persist.vendor.ims.playout_delay", "10");
        OverrideProperty("persist.vendor.ims.cam_sensor_delay", "20");
        OverrideProperty("persist.vendor.ims.display_delay", "40");
    }
}

void vendor_load_properties() {
    OverrideMemoryProperties();
    OverrideCarrierProperties();
}
