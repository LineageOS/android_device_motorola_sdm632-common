#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

import extract_utils.tools

extract_utils.tools.DEFAULT_PATCHELF_VERSION = '0_9'

from extract_utils.file import File
from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove,
    lib_fixups,
    lib_fixups_user_type,
    lib_fixup_remove_arch_suffix,
    lib_fixup_vendorcompat,
    lib_fixups_user_type,
    libs_clang_rt_ubsan,
    libs_proto_3_9_1,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
        'device/motorola/sdm632-common',
        'hardware/qcom-caf/msm8996',
        'hardware/qcom-caf/wlan',
        'vendor/qcom/opensource/dataservices',
]

def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None

lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'com.qualcomm.qti.imscmservice@1.0',
        'com.qualcomm.qti.imscmservice@2.0',
        'com.qualcomm.qti.imscmservice@2.1',
        'com.qualcomm.qti.imscmservice@2.2',
        'com.qualcomm.qti.uceservice@2.0',
        'com.qualcomm.qti.uceservice@2.1',
        'vendor.qti.data.factory@2.1',
        'vendor.qti.data.slm@1.0',
        'vendor.qti.hardware.fm@1.0',
        'vendor.qti.hardware.iop@1.0',
        'vendor.qti.hardware.iop@2.0',
        'vendor.qti.hardware.scve.objecttracker@1.0',
        'vendor.qti.hardware.scve.panorama@1.0',
        'vendor.qti.ims.callinfo@1.0',
        'vendor.qti.ims.rcsconfig@1.0',
        'vendor.qti.ims.rcsconfig@1.1',
        'vendor.qti.imsrtpservice@2.0',
        'vendor.qti.imsrtpservice@2.1',
        'vendor.qti.imsrtpservice@2.0'
    ): lib_fixup_vendor_suffix,
    (
        'libqsap_sdk',
        'libqsap_shim',
        'libmm-omxcore',
        'libc2dcolorconvert',
        'libkeymasterdeviceutils',
        'libkeymasterprovision',
        'libwpa_client'
    ): lib_fixup_remove,
}

blob_fixups: blob_fixups_user_type = {
    'vendor/app/Test.apk': blob_fixup()
        .apktool_patch('blob-patches/TestApk.patch', '-s'),
    'vendor/etc/test.conf': blob_fixup()
        .patch_file('blob-patches/TestConf.patch')
        .regex_replace('(LOG_.*_ENABLED)=1', '\\1=0')
        .add_line_if_missing('DEBUG=0'),
    ('vendor/etc/test.0.xml', 'vendor/etc/test.1.xml'): blob_fixup()
        .fix_xml(),
    'vendor/lib/test.so': blob_fixup()
        .patchelf_version('0_17_2')
        .fix_soname()
        .add_needed('to_add.so')
        .remove_needed('to_remove.so')
        .replace_needed('from.so', 'to.so')
        .clear_symbol_version('rpc_call_invoke')
        .strip_debug_sections()
        .binary_regex_replace(b'\xFF\x00\x00\x94', b'\xFE\x00\x00\x94')
        .sig_replace('C0 03 5F D6 ?? ?? ?? ?? C0 03 5F D6', '1F 20 03 D5')
        .call(blob_fixup_return_1, 'license_check'),

    (
        'system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.0-java.xml',
        'system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.1-java.xml',
        'system_ext/etc/permissions/qcrilhook.xml',
        'system_ext/etc/permissions/telephonyservice.xml',
        'system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml',
        'system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml',
    ): blob_fixup()
        .regex_replace('name="\/product\/framework', 'name="\/system_ext\/framework')
        .regex_replace('xml version="2.0"', 'xml version="1.0"'),
    (
        'system_ext/lib/lib-imscamera.so',
        'system_ext/lib/lib-imsvideocodec.so',
        'system_ext/lib64/lib-imscamera.so',
        'system_ext/lib64/lib-imsvideocodec.so',
    ): blob_fixup()
        .add_needed('libgui_shim.so'),
    'vendor/bin/pm-service': blob_fixup()
        .add_needed('libutils-v33.so'),
    'vendor/etc/permissions/com.motorola.motosignature.xml': blob_fixup()
        .regex_replace('name="\/system\/framework', 'name="\/vendor\/framework'),
    ('vendor/lib/sensors.rp.so', 'vendor/lib64/sensors.rp.so'): blob_fixup()
        .replace_needed('libutils.so', 'libutils-v32.so'),
    'vendor/lib64/libmdmcutback.so': blob_fixup()
        .add_needed('libqsap_shim.so'),
    'vendor/lib64/libmotext_inf.so': blob_fixup()
        .remove_needed('libril.so'),
    'vendor/lib64/libril-qc-hal-qmi.so': blob_fixup()
        .add_needed('libcutils_shim.so'),
    'vendor/lib64/libwvhidl.so': blob_fixup()
        .add_needed('libcrypto_shim.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'sdm632-common',
    'motorola',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
