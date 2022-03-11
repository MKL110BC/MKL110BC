/*!
 * @file      smtc_hal_gpio_pin_names.h
 *
 * @brief     Defines NucleoL073 platform pin names
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SMTC_HAL_GPIO_PIN_NAMES_H
#define SMTC_HAL_GPIO_PIN_NAMES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*!
 * @brief HAL Gpio pin name
 */
typedef enum hal_gpio_pin_names_e
{
	P0_0 = 0,
	P0_1 = 1,
	P0_2 = 2,
	P0_3 = 3,
	P0_4 = 4,
	P0_5 = 5,
	P0_6 = 6,
	P0_7 = 7,
	P0_8 = 8,
	P0_9 = 9,
	P0_10 = 10,
	P0_11 = 11,
	P0_12 = 12,
	P0_13 = 13,
	P0_14 = 14,
	P0_15 = 15,
	P0_16 = 16,
	P0_17 = 17,
	P0_18 = 18,
	P0_19 = 19,
	P0_20 = 20,
	P0_21 = 21,
	P0_22 = 22,
	P0_23 = 23,
	P0_24 = 24,
	P0_25 = 25,
	P0_26 = 26,
	P0_27 = 27,
	P0_28 = 28,
	P0_29 = 29,
	P0_30 = 30,
	P0_31 = 31,
	P1_0 = 32,
	P1_1 = 33,
	P1_2 = 34,
	P1_3 = 35,
	P1_4 = 36,
	P1_5 = 37,
	P1_6 = 38,
	P1_7 = 39,
	P1_8 = 40,
	P1_9 = 41,
	P1_10 = 42,
	P1_11 = 43,
	P1_12 = 44,
	P1_13 = 45,
	P1_14 = 46,
	P1_15 = 47,

	PIN_MAX
		
} hal_gpio_pin_names_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif  // SMTC_HAL_GPIO_PIN_NAMES_H
