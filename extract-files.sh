#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_COMMON=
ONLY_TARGET=
KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        --only-common )
                ONLY_COMMON=true
                ;;
        --only-target )
                ONLY_TARGET=true
                ;;
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        system_ext/etc/permissions/qcrilhook.xml|system_ext/etc/permissions/telephonyservice.xml)
            sed -i "s/\/product\/framework\//\/system_ext\/framework\//g" "${2}"
            ;;
        # Fix camera recording
        vendor/lib/libmmcamera2_pproc_modules.so)
            sed -i "s/ro.product.manufacturer/ro.product.nopefacturer/" "${2}"
            ;;
        # Fix missing symbols
        vendor/lib64/libril-qc-hal-qmi.so)
            for  LIBRIL_SHIM in $(grep -L "libcutils_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libcutils_shim.so" "$LIBRIL_SHIM"
            done
            ;;
        # Fix xml version
        system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml | system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml | system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.0-java.xml | system_ext/etc/permissions/com.qualcomm.qti.imscmservice-V2.1-java.xml)
            sed -i 's/xml version="2.0"/xml version="1.0"/' "${2}"
            sed -i "s/\/product\/framework\//\/system_ext\/framework\//g" "${2}"
            ;;
        # Fix missing symbols
        product/lib64/lib-imscamera.so | product/lib64/lib-imsvideocodec.so | product/lib/lib-imscamera.so | product/lib/lib-imsvideocodec.so)
            for LIBGUI_SHIM in $(grep -L "libgui_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libgui_shim.so" "${LIBGUI_SHIM}"
            done
            ;;
        # memset shim
        vendor/bin/charge_only_mode)
            for  LIBMEMSET_SHIM in $(grep -L "libmemset_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libmemset_shim.so" "$LIBMEMSET_SHIM"
            done
            ;;
        # Fix missing symbols
        vendor/bin/pm-service)
            grep -q libutils-v33.so "${2}" || "${PATCHELF}" --add-needed "libutils-v33.so" "${2}"
            ;;
        # Move to vendor
        vendor/etc/permissions/com.motorola.motosignature.xml)
            sed -i 's|/system/framework|/vendor/framework|' "${2}"
            ;;
        # Fix missing symbols
        vendor/lib/libmot_gpu_mapper.so)
            for LIBGUI_SHIM in $(grep -L "libgui_shim_vendor.so" "${2}"); do
                "${PATCHELF}" --add-needed "libgui_shim_vendor.so" "${LIBGUI_SHIM}"
            done
            ;;
        # qsap shim
        vendor/lib64/libmdmcutback.so)
            for  LIBQSAP_SHIM in $(grep -L "libqsap_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libqsap_shim.so" "$LIBQSAP_SHIM"
            done
            ;;
        # libutils-v32
        vendor/lib/soundfx/libspeakerbundle.so | vendor/lib/sensors.rp.so | vendor/lib64/sensors.rp.so)
            "${PATCHELF}" --replace-needed libutils.so libutils-v32.so "${2}"
            ;;
    esac
}

if [ -z "${ONLY_TARGET}" ]; then
    # Initialize the helper for common device
    setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

    extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

if [ -z "${ONLY_COMMON}" ] && [ -s "${MY_DIR}/../${DEVICE}/proprietary-files.txt" ]; then
    # Reinitialize the helper for device
    source "${MY_DIR}/../${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

    extract "${MY_DIR}/../${DEVICE}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

"${MY_DIR}/setup-makefiles.sh"
