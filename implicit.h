#ifndef _ETL_IMPLICIT_H_INCLUDED
#define _ETL_IMPLICIT_H_INCLUDED

/**
 * @addtogroup preprocessor
 * @{
 */

/** @file
 *
 * Provides the #ETL_IMPLICIT macro.
 */

/**
 * Explicit marker for `implicit` operations.
 *
 * This do-nothing macro serves as a reminder that an operation is *not*
 * explicit.  It exists because its author wishes C++ required things to
 * be explicitly marked implicit.
 */
#define ETL_IMPLICIT /* empty */

/**@}*/

#endif  // _ETL_IMPLICIT_H_INCLUDED
