#ifndef ITH_IOEX_H
#define ITH_IOEX_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_ioex IoEx
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ioex port definition.
 */

#define GPIO_MAX_MAPPING_COUNT 16

typedef struct
{
    uint8_t MappingCount;                            ///< MappingCount
    uint8_t MappingGPIONum[GPIO_MAX_MAPPING_COUNT];  ///< MappingGPIONum
    uint8_t LevelTrigger;
    uint8_t EdgeTrigger;
    uint8_t BothEdge;
    uint8_t SigleEdge;
    uint8_t RisingTrigger;
    uint8_t FallingTrigger;
} ITHIOEXConfig;

#ifdef __cplusplus
}
#endif

#endif // ITH_IOEX_H
/** @} */ // end of ith_ioex
/** @} */ // end of ith