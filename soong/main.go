package sdm632

import (
    "android/soong/android"
)

func init() {
    android.RegisterModuleType("motorola_sdm632_init_library_static", initLibraryFactory)
}
