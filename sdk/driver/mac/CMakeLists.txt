include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/mac/inc/
    ${PROJECT_SOURCE_DIR}/sdk/driver/mac/gmac/
)

add_library(mac STATIC
    mii.c
    gmac/skb.c
    gmac/ite_gmac.c
)
