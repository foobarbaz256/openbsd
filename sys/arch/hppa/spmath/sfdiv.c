/*	$OpenBSD: sfdiv.c,v 1.4 2001/03/29 03:58:19 mickey Exp $	*/

/*
 * Copyright 1996 1995 by Open Software Foundation, Inc.
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * pmk1.1
 */
/*
 * (c) Copyright 1986 HEWLETT-PACKARD COMPANY
 *
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *     permission to use, copy, modify, and distribute this file
 * for any purpose is hereby granted without fee, provided that
 * the above copyright notice and this notice appears in all
 * copies, and that the name of Hewlett-Packard Company not be
 * used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * Hewlett-Packard Company makes no representations about the
 * suitability of this software for any purpose.
 */

#include "../spmath/float.h"
#include "../spmath/sgl_float.h"

/*
 *  Single Precision Floating-point Divide
 */
int
sgl_fdiv(srcptr1,srcptr2,dstptr,status)

sgl_floating_point *srcptr1, *srcptr2, *dstptr;
unsigned int *status;
{
	register unsigned int opnd1, opnd2, opnd3, result;
	register int dest_exponent, count;
	register int inexact = FALSE, guardbit = FALSE, stickybit = FALSE;
	int is_tiny;

	opnd1 = *srcptr1;
	opnd2 = *srcptr2;
	/*
	 * set sign bit of result
	 */
	if (Sgl_sign(opnd1) ^ Sgl_sign(opnd2)) Sgl_setnegativezero(result);
	else Sgl_setzero(result);
	/*
	 * check first operand for NaN's or infinity
	 */
	if (Sgl_isinfinity_exponent(opnd1)) {
		if (Sgl_iszero_mantissa(opnd1)) {
			if (Sgl_isnotnan(opnd2)) {
				if (Sgl_isinfinity(opnd2)) {
					/*
					 * invalid since both operands
					 * are infinity
					 */
					if (Is_invalidtrap_enabled())
						return(INVALIDEXCEPTION);
					Set_invalidflag();
					Sgl_makequietnan(result);
					*dstptr = result;
					return(NOEXCEPTION);
				}
				/*
				 * return infinity
				 */
				Sgl_setinfinity_exponentmantissa(result);
				*dstptr = result;
				return(NOEXCEPTION);
			}
		}
		else {
			/*
			 * is NaN; signaling or quiet?
			 */
			if (Sgl_isone_signaling(opnd1)) {
				/* trap if INVALIDTRAP enabled */
				if (Is_invalidtrap_enabled())
					return(INVALIDEXCEPTION);
				/* make NaN quiet */
				Set_invalidflag();
				Sgl_set_quiet(opnd1);
			}
			/*
			 * is second operand a signaling NaN?
			 */
			else if (Sgl_is_signalingnan(opnd2)) {
				/* trap if INVALIDTRAP enabled */
				if (Is_invalidtrap_enabled())
					return(INVALIDEXCEPTION);
				/* make NaN quiet */
				Set_invalidflag();
				Sgl_set_quiet(opnd2);
				*dstptr = opnd2;
				return(NOEXCEPTION);
			}
			/*
			 * return quiet NaN
			 */
			*dstptr = opnd1;
			return(NOEXCEPTION);
		}
	}
	/*
	 * check second operand for NaN's or infinity
	 */
	if (Sgl_isinfinity_exponent(opnd2)) {
		if (Sgl_iszero_mantissa(opnd2)) {
			/*
			 * return zero
			 */
			Sgl_setzero_exponentmantissa(result);
			*dstptr = result;
			return(NOEXCEPTION);
		}
		/*
		 * is NaN; signaling or quiet?
		 */
		if (Sgl_isone_signaling(opnd2)) {
			/* trap if INVALIDTRAP enabled */
			if (Is_invalidtrap_enabled()) return(INVALIDEXCEPTION);
			/* make NaN quiet */
			Set_invalidflag();
			Sgl_set_quiet(opnd2);
		}
		/*
		 * return quiet NaN
		 */
		*dstptr = opnd2;
		return(NOEXCEPTION);
	}
	/*
	 * check for division by zero
	 */
	if (Sgl_iszero_exponentmantissa(opnd2)) {
		if (Sgl_iszero_exponentmantissa(opnd1)) {
			/* invalid since both operands are zero */
			if (Is_invalidtrap_enabled()) return(INVALIDEXCEPTION);
			Set_invalidflag();
			Sgl_makequietnan(result);
			*dstptr = result;
			return(NOEXCEPTION);
		}
		if (Is_divisionbyzerotrap_enabled())
			return(DIVISIONBYZEROEXCEPTION);
		Set_divisionbyzeroflag();
		Sgl_setinfinity_exponentmantissa(result);
		*dstptr = result;
		return(NOEXCEPTION);
	}
	/*
	 * Generate exponent
	 */
	dest_exponent = Sgl_exponent(opnd1) - Sgl_exponent(opnd2) + SGL_BIAS;

	/*
	 * Generate mantissa
	 */
	if (Sgl_isnotzero_exponent(opnd1)) {
		/* set hidden bit */
		Sgl_clear_signexponent_set_hidden(opnd1);
	}
	else {
		/* check for zero */
		if (Sgl_iszero_mantissa(opnd1)) {
			Sgl_setzero_exponentmantissa(result);
			*dstptr = result;
			return(NOEXCEPTION);
		}
		/* is denormalized; want to normalize */
		Sgl_clear_signexponent(opnd1);
		Sgl_leftshiftby1(opnd1);
		Sgl_normalize(opnd1,dest_exponent);
	}
	/* opnd2 needs to have hidden bit set with msb in hidden bit */
	if (Sgl_isnotzero_exponent(opnd2)) {
		Sgl_clear_signexponent_set_hidden(opnd2);
	}
	else {
		/* is denormalized; want to normalize */
		Sgl_clear_signexponent(opnd2);
		Sgl_leftshiftby1(opnd2);
		while(Sgl_iszero_hiddenhigh7mantissa(opnd2)) {
			Sgl_leftshiftby8(opnd2);
			dest_exponent += 8;
		}
		if(Sgl_iszero_hiddenhigh3mantissa(opnd2)) {
			Sgl_leftshiftby4(opnd2);
			dest_exponent += 4;
		}
		while(Sgl_iszero_hidden(opnd2)) {
			Sgl_leftshiftby1(opnd2);
			dest_exponent += 1;
		}
	}

	/* Divide the source mantissas */

	/*
	 * A non_restoring divide algorithm is used.
	 */
	Sgl_subtract(opnd1,opnd2,opnd1);
	Sgl_setzero(opnd3);
	for (count=1;count<=SGL_P && Sgl_all(opnd1);count++) {
		Sgl_leftshiftby1(opnd1);
		Sgl_leftshiftby1(opnd3);
		if (Sgl_iszero_sign(opnd1)) {
			Sgl_setone_lowmantissa(opnd3);
			Sgl_subtract(opnd1,opnd2,opnd1);
		}
		else Sgl_addition(opnd1,opnd2,opnd1);
	}
	if (count <= SGL_P) {
		Sgl_leftshiftby1(opnd3);
		Sgl_setone_lowmantissa(opnd3);
		Sgl_leftshift(opnd3,SGL_P-count);
		if (Sgl_iszero_hidden(opnd3)) {
			Sgl_leftshiftby1(opnd3);
			dest_exponent--;
		}
	}
	else {
		if (Sgl_iszero_hidden(opnd3)) {
			/* need to get one more bit of result */
			Sgl_leftshiftby1(opnd1);
			Sgl_leftshiftby1(opnd3);
			if (Sgl_iszero_sign(opnd1)) {
				Sgl_setone_lowmantissa(opnd3);
				Sgl_subtract(opnd1,opnd2,opnd1);
			}
			else Sgl_addition(opnd1,opnd2,opnd1);
			dest_exponent--;
		}
		if (Sgl_iszero_sign(opnd1)) guardbit = TRUE;
		stickybit = Sgl_all(opnd1);
	}
	inexact = guardbit | stickybit;

	/*
	 * round result
	 */
	if (inexact && (dest_exponent > 0 || Is_underflowtrap_enabled())) {
		Sgl_clear_signexponent(opnd3);
		switch (Rounding_mode()) {
			case ROUNDPLUS:
				if (Sgl_iszero_sign(result))
					Sgl_increment_mantissa(opnd3);
				break;
			case ROUNDMINUS:
				if (Sgl_isone_sign(result))
					Sgl_increment_mantissa(opnd3);
				break;
			case ROUNDNEAREST:
				if (guardbit &&
				    (stickybit || Sgl_isone_lowmantissa(opnd3)))
					Sgl_increment_mantissa(opnd3);
		}
		if (Sgl_isone_hidden(opnd3)) dest_exponent++;
	}
	Sgl_set_mantissa(result,opnd3);

	/*
	 * Test for overflow
	 */
	if (dest_exponent >= SGL_INFINITY_EXPONENT) {
		/* trap if OVERFLOWTRAP enabled */
		if (Is_overflowtrap_enabled()) {
			/*
			 * Adjust bias of result
			 */
			Sgl_setwrapped_exponent(result,dest_exponent,ovfl);
			*dstptr = result;
			if (inexact) {
			    if (Is_inexacttrap_enabled())
				return(OVERFLOWEXCEPTION | INEXACTEXCEPTION);
			    else Set_inexactflag();
			}
			return(OVERFLOWEXCEPTION);
		}
		Set_overflowflag();
		/* set result to infinity or largest number */
		Sgl_setoverflow(result);
		inexact = TRUE;
	}
	/*
	 * Test for underflow
	 */
	else if (dest_exponent <= 0) {
		/* trap if UNDERFLOWTRAP enabled */
		if (Is_underflowtrap_enabled()) {
			/*
			 * Adjust bias of result
			 */
			Sgl_setwrapped_exponent(result,dest_exponent,unfl);
			*dstptr = result;
			if (inexact) {
			    if (Is_inexacttrap_enabled())
				return(UNDERFLOWEXCEPTION | INEXACTEXCEPTION);
			    else Set_inexactflag();
			}
			return(UNDERFLOWEXCEPTION);
		}

		/* Determine if should set underflow flag */
		is_tiny = TRUE;
		if (dest_exponent == 0 && inexact) {
			switch (Rounding_mode()) {
			case ROUNDPLUS:
				if (Sgl_iszero_sign(result)) {
					Sgl_increment(opnd3);
					if (Sgl_isone_hiddenoverflow(opnd3))
						is_tiny = FALSE;
					Sgl_decrement(opnd3);
				}
				break;
			case ROUNDMINUS:
				if (Sgl_isone_sign(result)) {
					Sgl_increment(opnd3);
					if (Sgl_isone_hiddenoverflow(opnd3))
						is_tiny = FALSE;
					Sgl_decrement(opnd3);
				}
				break;
			case ROUNDNEAREST:
				if (guardbit && (stickybit ||
				    Sgl_isone_lowmantissa(opnd3))) {
					Sgl_increment(opnd3);
					if (Sgl_isone_hiddenoverflow(opnd3))
						is_tiny = FALSE;
					Sgl_decrement(opnd3);
				}
				break;
			}
		}

		/*
		 * denormalize result or set to signed zero
		 */
		stickybit = inexact;
		Sgl_denormalize(opnd3,dest_exponent,guardbit,stickybit,inexact);

		/* return rounded number */
		if (inexact) {
			switch (Rounding_mode()) {
			case ROUNDPLUS:
				if (Sgl_iszero_sign(result)) {
					Sgl_increment(opnd3);
				}
				break;
			case ROUNDMINUS:
				if (Sgl_isone_sign(result))  {
					Sgl_increment(opnd3);
				}
				break;
			case ROUNDNEAREST:
				if (guardbit && (stickybit ||
				    Sgl_isone_lowmantissa(opnd3))) {
					Sgl_increment(opnd3);
				}
				break;
			}
			if (is_tiny) Set_underflowflag();
		}
		Sgl_set_exponentmantissa(result,opnd3);
	}
	else Sgl_set_exponent(result,dest_exponent);
	*dstptr = result;
	/* check for inexact */
	if (inexact) {
		if (Is_inexacttrap_enabled()) return(INEXACTEXCEPTION);
		else  Set_inexactflag();
	}
	return(NOEXCEPTION);
}
