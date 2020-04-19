#ifndef	_LIBCRCCFG_H
#define _LIBCRCCFG_H
/**
 * Most desirable polinomials picked from the study 
http://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf
by Philip Koopman, Carnegie Mellon University, Pittsburgh 
 *
 **/
/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

#define CRC8_USE_TABLES  /* Table implementation faster, use more RAM */

/* CRC8 used in Sensirion SFM3000 series I2C Flow sensors */
#define CRC8_SENSIRION      0x131   /* P(x) = x^8 + x^5 + x^4 + 1 = 100110001 */
/* CRC8 polynomial CRC-8, 0xEA, good results ONLY up to 85 bits */
#define CRC8_CRC8           0x1D5   /* P(x) = x^8 + x^7 + x^6 + x^4 + x^2 + 1 */
/* CRC8 polynomial 0xA6, best for lengths > 120 bits (>15 bytes) */
#define CRC8_LONG           0x14D   /* P(x) = x^8 + x^6 + x^3 + x^2 + 1 */
/* CRC8 polynomial C2, 0x97 Baicheva98, best for < 119 bits, best compromise overall */
#define CRC8_BAICHEVA       0x12F   /* P(x) = x^8 + x^5 + x^3 + x^2 + x + 1 */

/* Select 1 polinomial for CRC8 from the ones above */
#define CRC8_POLYNOMIAL     CRC8_BAICHEVA

#endif
