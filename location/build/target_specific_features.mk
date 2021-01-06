GNSS_CFLAGS := \
    -Werror \
    -Wno-unused-parameter \
    -Wno-macro-redefined \
    -Wno-error=reorder \
    -Wno-reorder-ctor \
    -Wno-error=missing-braces \
    -Wno-error=self-assign \
    -Wno-enum-conversion \
    -Wno-logical-op-parentheses \
    -Wno-error=null-arithmetic \
    -Wno-error=null-conversion \
    -Wno-error=parentheses-equality \
    -Wno-undefined-bool-conversion \
    -Wno-error=tautological-compare \
    -Wno-error=switch \
    -Wno-error=date-time

# Activate the following two lines for regression testing
#GNSS_SANITIZE := address cfi alignment bounds null unreachable integer
#GNSS_SANITIZE_DIAG := address cfi alignment bounds null unreachable integer
