#ifndef ITH_REG_H
#define ITH_REG_H

#include "ite/mock_helper.h"
#include "ith/ith_platform.h"

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_reg Register
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    uint16_t ithReadRegH(uint16_t addr);
    DECLARE_COULD_BE_MOCKED_FUNC1(uint16_t, ithReadRegH, uint16_t);

    void ithWriteRegH(uint16_t addr, uint16_t data);
    DECLARE_COULD_BE_MOCKED_FUNC2(void, ithWriteRegH, uint16_t, uint16_t);

    uint16_t ithReadShortRegA(uint32_t addr);
    uint32_t ithReadRegA(uint32_t addr);
    void ithWriteRegA(uint32_t addr, uint32_t data);

    uint8_t ithReadReg8(uint32_t addr);
    void ithWriteReg8(uint32_t addr, uint8_t data);

    uint16_t ithReadReg16(uint32_t addr);
    void ithWriteReg16(uint32_t addr, uint16_t data);

    #define ITH_READ_REG_H(addr)
    #define ITH_WRITE_REG_H(addr, data)

#elif defined(__LINUX_ARM_ARCH__)
    static inline uint16_t ithReadRegH(uint16_t addr)
    {
        return (uint16_t)readl(ITH_REG_BASE + ITH_HOST_BASE + addr);
    }

    static inline void ithWriteRegH(uint16_t addr, uint16_t data)
    {
        writel(ITH_REG_BASE + ITH_HOST_BASE + addr, (uint32_t)data);
    }

    static inline uint32_t ithReadRegA(uint32_t addr)
    {
        return readl(ITH_REG_BASE + addr);
    }

    static inline void ithWriteRegA(uint32_t addr, uint32_t data)
    {
        writel(ITH_REG_BASE + addr, data);
    }

    #define ITH_READ_REG_H(addr) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)); \
        } while (0)

    #define ITH_WRITE_REG_H(addr, data) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)) = (data); \
        } while (0)

#else
    /**
     * Reads host register value.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint16_t ithReadRegH(uint16_t addr)
    {
        return *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + addr);
    }

    /**
     * Writes host register value.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteRegH(uint16_t addr, uint16_t data)
    {
        *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + addr) = data;
    }

    /**
     * Reads amba register value.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint32_t ithReadRegA(uint32_t addr)
    {
        return *(uint32_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteRegA(uint32_t addr, uint32_t data)
    {
        *(uint32_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads amba register value, in byte.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint8_t ithReadReg8(uint32_t addr)
    {
        return *(uint8_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value, in byte.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteReg8(uint32_t addr, uint8_t data)
    {
        *(uint8_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads amba register value, in 16-bit.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint16_t ithReadReg16(uint32_t addr)
    {
        return *(uint16_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value, in 16-bit.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteReg16(uint32_t addr, uint16_t data)
    {
        *(uint16_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads host register value macro.
     *
     * @param addr The register address.
     * @return The register value.
     */
    #define ITH_READ_REG_H(addr) \
        *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr))

    /**
     * Writes host register value macro.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    #define ITH_WRITE_REG_H(addr, data) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)) = (data); \
        } while (0)

    /**
     * Reads AMBA register value macro.
     *
     * @param addr The register address.
     * @return The register value.
     */
    #define ITH_READ_REG_A(addr) \
        *(uint32_t volatile*)(ITH_REG_BASE + (addr))

    /**
     * Writes AMBA register value macro.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    #define ITH_WRITE_REG_A(addr, data) \
        do { \
            *(uint32_t volatile*)(ITH_REG_BASE + (addr)) = (data); \
        } while (0)

#endif // _WIN32

/**
 * Writes host register value with mask.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
__attribute__((flatten))
static inline void ithWriteRegMaskH(uint16_t addr, uint16_t data, uint16_t mask)
{
    ithWriteRegH(addr, (ithReadRegH(addr) & ~mask) | (data & mask));
}

/**
 * Sets host register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
__attribute__((flatten))
static inline void ithSetRegBitH(uint16_t addr, unsigned int bit)
{
    ithWriteRegMaskH(addr, 0x1 << bit, 0x1 << bit);
}

/**
 * Clears host register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
__attribute__((flatten))
static inline void ithClearRegBitH(uint16_t addr, unsigned int bit)
{
    ithWriteRegMaskH(addr, 0, 0x1 << bit);
}

/**
 * Writes AMBA register value with mask.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
__attribute__((flatten))
static inline void ithWriteRegMaskA(uint32_t addr, uint32_t data, uint32_t mask)
{
    ithWriteRegA(addr, (ithReadRegA(addr) & ~mask) | (data & mask));
}

/**
 * Sets AMBA register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
__attribute__((flatten))
static inline void ithSetRegBitA(uint32_t addr, unsigned int bit)
{
    ithWriteRegMaskA(addr, 0x1 << bit, 0x1 << bit);
}

/**
 * Clears AMBA register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
__attribute__((flatten))
static inline void ithClearRegBitA(uint32_t addr, unsigned int bit)
{
    ithWriteRegMaskA(addr, 0, 0x1 << bit);
}

/**
 * Writes host register value with mask macro.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
#define ITH_WRITE_REG_MASK_H(addr, data, mask) \
    do { \
        ITH_WRITE_REG_H((addr), (ITH_READ_REG_H(addr) & ~(mask)) | ((data) & (mask))); \
    } while (0)

/**
 * Sets host register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
#define ITH_SET_REG_BIT_H(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_H((addr), 0x1 << (bit), 0x1 << (bit)); \
    } while (0)

/**
 * Clears host register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
#define ITH_CLEAR_REG_BIT_H(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_H((addr), 0, 0x1 << (bit)); \
    } while (0)

/**
 * Writes AMBA register value with mask macro.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
#define ITH_WRITE_REG_MASK_A(addr, data, mask) \
    do { \
        ITH_WRITE_REG_A((addr), (ITH_READ_REG_A(addr) & ~(mask)) | ((data) & (mask))); \
    } while (0)

/**
 * Sets AMBA register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
#define ITH_SET_REG_BIT_A(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_A((addr), 0x1 << (bit), 0x1 << (bit)); \
    } while (0)

/**
 * Clears AMBA register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
#define ITH_CLEAR_REG_BIT_A(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_A((addr), 0, 0x1 << (bit)); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // ITH_REG_H
/** @} */ // end of ith_reg
/** @} */ // end of ith