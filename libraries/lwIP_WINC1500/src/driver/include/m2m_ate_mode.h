/**
 *
 * \file
 *
 * \brief WINC ATE Test Driver Interface.
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifdef _M2M_ATE_FW_

#ifndef _M2M_ATE_MODE_H_
#define _M2M_ATE_MODE_H_

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#include "common/include/nm_common.h"
#include "driver/include/m2m_types.h"

/** \defgroup m2m_ate ATE
*/
/**@defgroup  ATEDefine Defines
 * @ingroup m2m_ate
 * @{
 */
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#define M2M_ATE_MAX_NUM_OF_RATES		(20)
/*!<
Maximum number of all rates (b,g and n)
 */
#define M2M_ATE_MAX_FRAME_LENGTH		(1024)
/*!< Maximum number of length for each frame
 */
#define M2M_ATE_MIN_FRAME_LENGTH		(1)
/*!< Minimum number of length for each frame
 */ 
#define M2M_ATE_SUCCESS					(M2M_SUCCESS)
/*!< No Error and operation has been completed successfully.
*/
#define M2M_ATE_ERR_VALIDATE			(M2M_ERR_FAIL)	
/*!< Error in parameters passed to functions.
 */
#define M2M_ATE_ERR_TX_ALREADY_RUNNING	(-1)
/*!< Error in starting a transmission test. Another test is already running and its not allowed to start another ATE test.
 */
#define M2M_ATE_ERR_RX_ALREADY_RUNNING	(-2)			
/*!< Error in starting a reception test. Another test is already running and its not allowed to start another ATE test.
 */
#define M2M_ATE_ERR_UNHANDLED_CASE		(-3)	
/*!< Invalid case.
 */
#define M2M_ATE_RX_DISABLE_DA          		0x0
/*!< Filter selection for received frames: Disable filtering received frames by the destination address.
 */
#define M2M_ATE_RX_ENABLE_DA          		0x1
/*!< Filter selection for received frames: Enable filtering received frames by the destination address.
 */
#define M2M_ATE_RX_DISABLE_SA          		0x0
/*!< Filter selection for received frames: Disable filtering received frames by the source address.
 */
#define M2M_ATE_RX_ENABLE_SA          		0x1
/*!< Filter selection for received frames: Enable filtering received frames by the source address.
 */
#define M2M_ATE_DISABLE_SELF_MACADDR       0x0
/*!<Disable setting a new mac address through the ATE test application and use the pre-set mac address in the firmware.
 */
#define M2M_ATE_SET_SELF_MACADDR         	0x1
/*!<Enable setting a new mac address through the ATE test application and use the pre-set mac address.
 */
#define M2M_ATE_TX_DUTY_MAX_VALUE	M2M_ATE_TX_DUTY_1
/*!< The maximum value of duty cycle
*/
#define M2M_ATE_TX_DUTY_MIN_VALUE	M2M_ATE_TX_DUTY_10
/*!< The minimum value of duty cycle
*/
  //@}
/**@defgroup  ATEDataTypes DataTypes
 * @ingroup m2m_ate
 * @{
 */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*!
 *@enum		tenuM2mAteFwState	
 *@brief	Enumeration used to change ATE firmware states
 */
typedef enum {
	M2M_ATE_FW_STATE_STOP			= 0x00,
	/*!< State to stop ATE firmware
	 */
	M2M_ATE_FW_STATE_RUN			= 0x01,
	/*!< State to run ATE firmware
	 */
}tenuM2mAteFwState;

/*!
 *@enum		tenuM2mAteTxRates	
 *@brief	Enumeration used to index the TX rates that can be used during the transmission test.
 */
typedef enum {
	M2M_ATE_TX_RATE_1_Mbps_INDEX	= 0x00,
	M2M_ATE_TX_RATE_2_Mbps_INDEX	= 0x01,
	M2M_ATE_TX_RATE_55_Mbps_INDEX	= 0x02,
	M2M_ATE_TX_RATE_11_Mbps_INDEX	= 0x03,
	/*!< B-Rates
	 */
	M2M_ATE_TX_RATE_6_Mbps_INDEX	= 0x04,
	M2M_ATE_TX_RATE_9_Mbps_INDEX	= 0x05,
	M2M_ATE_TX_RATE_12_Mbps_INDEX	= 0x06,
	M2M_ATE_TX_RATE_18_Mbps_INDEX	= 0x07,
	M2M_ATE_TX_RATE_24_Mbps_INDEX	= 0x08,
	M2M_ATE_TX_RATE_36_Mbps_INDEX	= 0x09,
	M2M_ATE_TX_RATE_48_Mbps_INDEX	= 0x0A,
	M2M_ATE_TX_RATE_54_Mbps_INDEX	= 0x0B,
	/*!< G-Rates
	 */
	M2M_ATE_TX_RATE_MCS_0_INDEX		= 0x0C,
	M2M_ATE_TX_RATE_MCS_1_INDEX		= 0x0D,
	M2M_ATE_TX_RATE_MCS_2_INDEX		= 0x0E,
	M2M_ATE_TX_RATE_MCS_3_INDEX		= 0x0F,
	M2M_ATE_TX_RATE_MCS_4_INDEX		= 0x10,
	M2M_ATE_TX_RATE_MCS_5_INDEX		= 0x11,
	M2M_ATE_TX_RATE_MCS_6_INDEX		= 0x12,
	M2M_ATE_TX_RATE_MCS_7_INDEX		= 0x13,
	/*!< N-Rates
	 */
}tenuM2mAteTxIndexOfRates;

/*!
 *@enum		tenuM2mAteTxDutyCycle	
 *@brief	Enumeration used to index the TX duty cycle that can be used during the transmission test.
 */
typedef enum {
	M2M_ATE_TX_DUTY_1				= 0x01,
	M2M_ATE_TX_DUTY_2				= 0x02,
	M2M_ATE_TX_DUTY_3				= 0x03,
	M2M_ATE_TX_DUTY_4				= 0x04,
	M2M_ATE_TX_DUTY_5				= 0x05,
	M2M_ATE_TX_DUTY_6				= 0x06,
	M2M_ATE_TX_DUTY_7				= 0x07,
	M2M_ATE_TX_DUTY_8				= 0x08,
	M2M_ATE_TX_DUTY_9				= 0x09,
	M2M_ATE_TX_DUTY_10				= 0xA0,
}tenuM2mAteTxDutyCycle;

/*!
 *@enum		tenuM2mAteTxDpdControl	
 *@brief	Enumeration for the allowed Digital-pre distortion(DPD) control values.
 */
typedef enum {
	M2M_ATE_TX_DPD_DYNAMIC	= 0x00,
	/*!< Dynamic mode indicates that DPD values will be set dynamically from a lookup table pre-set with the DPD coefficents.
	*/
	M2M_ATE_TX_DPD_BYPASS	= 0x01,
	/*!< Bypass mode indicates that the DPD control will be bypassed.
	*/
	M2M_ATE_TX_DPD_ENABLED	= 0x02,
	/*!< Enabled mode allows the tester to manually set the DPD coefficients.
	*/
}tenuM2mAteTxDpdControl;

/*!
 *@enum		tenuM2mAteTxGainSetting	
 *@brief	Enumeration for the allowed TX gain selection modes.
 */
typedef enum {
	M2M_ATE_TX_GAIN_DYNAMIC	= 0x00,
	/*!< Dynamic mode indicates that Tx gain values for the digital gain,pa and ppa, will be set dynamically from a lookup table based on the Tx_rate configured.
	*/
	M2M_ATE_TX_GAIN_BYPASS	= 0x01,
	/*!< Bypass mode indicates that Tx gain configurations will be bypassed.
	*/
	M2M_ATE_TX_GAIN_FCC		= 0x02,
	/*!< Using the FCC tx gain configuration indicates that the tx gain values will be used from the FCC flashed table(pre-configured values from a customer).
	*/
	M2M_ATE_TX_GAIN_TELEC	= 0x03,
	/*!< Using the TELEC tx gain configuration indicates that the tx gain values will be used from the TELEC flashed table(pre-configured values from a customer).
	*/
}tenuM2mAteTxGainSetting;

/*!
 *@enum		tenuM2mAtePMUSetting	
 *@brief	Used to Enable PMU or disable it
 */
typedef enum {
	M2M_ATE_PMU_DISBLE	= 0x00,
	/*!< Disable using PMU mode
	*/
	M2M_ATE_PMU_ENABLE	= 0x01,
	/*!< Enable using PMU mode
	*/
}tenuM2mAtePMUSetting;

/*!
 *@enum	 tenuM2mAteTxSource	
 *@brief	Used to define the Tx source, either PHY mode or MAC mode.
 */
typedef enum {
	M2M_ATE_TX_SRC_MAC	= 0x00,
	/*!< When the TX Source is set to MAC, it indicates that the TX frames are manually framed and sent from the MAC layer
	*/
	M2M_ATE_TX_SRC_PHY	= 0x01,
	/*!< When the TX source is set to PHY, it indicates that transmission sequence occurs from PHY layer in the form of  pulses
	*/
}tenuM2mAteTxSource;

/*!
 *@enum	tenuM2mAteTxMode	
 *@brief	Used to define the mode of PHY TX transmission source: Continuous Wave(CW) or Normal(i.e CW is disabled) TX sequence
 */
typedef enum {
	M2M_ATE_TX_MODE_NORM	= 0x00,
	/*!< When the TX source is set to PHY,normal mode indicates that continous transmission is disabled. 
	*/
	M2M_ATE_TX_MODE_CW		= 0x01,
	/*!< When the TX source is set to PHY, continous mode indicates that transmission sequences occur back to back in a continous wave from the PHY layer.
	*/
}tenuM2mAteTxMode;

/*!
 *@enum		tenuM2mAteRxPwrMode	
 *@brief	Used to define type of RX mode either high power or low power
 */
typedef enum {
	M2M_ATE_RX_PWR_HIGH	= 0x00,
	/*!< Indicates that receive mode is operating at high power
	*/	
	M2M_ATE_RX_PWR_LOW	= 0x01,
	/*!< Indicates that receive mode is operating at low power
	*/
}tenuM2mAteRxPwrMode;

/*!
 *@enum		tenuM2mAteChannels	
 *@brief	Available channels for TX and RX in the 2.4GHz spectrum starting at 2412MHz with a 5MHz bandwidth.
 */
typedef enum {
	M2M_ATE_CHANNEL_1	= 0x01,
	/*!< Channel 1: 2412MHz
	 */
	M2M_ATE_CHANNEL_2	= 0x02,
	/*!< Channel 2: 2417MHz
	 */
	M2M_ATE_CHANNEL_3	= 0x03,
	/*!< Channel 3: 2422MHz
	 */
	M2M_ATE_CHANNEL_4	= 0x04,
	/*!< Channel 4: 2427MHz
	 */
	M2M_ATE_CHANNEL_5	= 0x05,
	/*!< Channel 5: 2432MHz
	 */
	M2M_ATE_CHANNEL_6	= 0x06,
	/*!< Channel 6: 2437MHz
	 */
	M2M_ATE_CHANNEL_7	= 0x07,
	/*!< Channel 7: 2442MHz
	 */
	M2M_ATE_CHANNEL_8	= 0x08,
	/*!< Channel 8: 2447MHz
	 */
	M2M_ATE_CHANNEL_9	= 0x09,
	/*!< Channel 9: 2452MHz
	 */
	M2M_ATE_CHANNEL_10	= 0x0A,
	/*!< Channel 10: 2462MHz
	 */
	M2M_ATE_CHANNEL_11	= 0x0B,
	/*!< Channel 11: 2467MHz
	 */
	M2M_ATE_CHANNEL_12	= 0x0C,
	/*!< Channel 12: 2472MHz
	 */
	M2M_ATE_CHANNEL_13	= 0x0D,
	/*!< Channel 13: 2472MHz
	 */
	M2M_ATE_CHANNEL_14	= 0x0E,
	/*!< Channel 14: 2484MHz
	 */
}tenuM2mAteChannels;

/*!
 *@struct	tstrM2mAteRxStatus
 *@brief	Used to save statistics for receive(RX) test case
 */
typedef struct {
	uint32 num_rx_pkts;
	/*!< Number of total RX packets
	 */
	uint32 num_err_pkts;
	/*!< Number of RX failed packets
	 */
	 uint32 num_good_pkts;
	/*!< Number of RX packets actually received
	 */
} tstrM2mAteRxStatus;

/*!
 *@struct	tstrM2mAteRxStatus
 *@brief	Used to save recieve test case configuration  
 *@see    tenuM2mAteRxPwrMode
 */
typedef struct {
	uint8 u8RxPwrMode;
	/*!< RX power mode review \ref tenuM2mAteRxPwrMode
	 */
} tstrM2mAteInit;

/*!
 *@struct	tstrM2mAteTx
 *@brief	Used for the transmission(Tx) test configuration.
 */
typedef struct {
	uint32	num_frames;	
	/*!< Number of frames to be sent where maximum number allowed is 4294967295 ul, and ZERO means infinite number of frames
	 */
	uint32	data_rate;
	/*!< Rate to send packets, to select a rate use values from the enumeration \ref tenuM2mAteTxIndexOfRates and pass it to \ref m2m_ate_get_tx_rate
	 */
	uint8	channel_num;
	/*!< Channel number as enumerated at \ref tenuM2mAteChannels
	 */
	uint8    duty_cycle; 
	/*!< Duty cycle value between from 1 to 10, where maximum = 1, minimum = 10. As enumerated \ref tenuM2mAteTxDutyCycle
	 */
	uint16    frame_len;
    /*!< Use @ref M2M_ATE_MAX_FRAME_LENGTH (1024) as the maximum value while @ref M2M_ATE_MIN_FRAME_LENGTH (1) is the minimum value
	 */
	uint8     tx_gain_sel;
	/*!< TX gain mode selection value \ref tenuM2mAteTxGainSetting
	 */
	uint8     dpd_ctrl;
	/*!< DPD mode value\ref tenuM2mAteTxDpdControl
	 */
	uint8     use_pmu;
	/*!< This is 0 if PMU is not used otherwise it must be be 1 \ref tenuM2mAtePMUSetting
	 */
	uint8     phy_burst_tx; 
	/*!< Source of Burst TX either PHY or MAC \ref tenuM2mAteTxSource
	 */
	uint8     cw_tx; 
	/*!< Mode of Phy TX transmission either normal TX sequence or CW(Continuous Wave) TX sequence \ref tenuM2mAteTxMode
	 */
	uint32     xo_offset_x1000; 
	/*!< Signed XO offset value in Part Per Million(PPM) multiplied by 1000.
	 */
	uint8     use_efuse_xo_offset;
	/*!< Set to 0 to use the XO offset provided in xo_offset_x1000. Set to 1 to use XO offset programmed on WINC efuse. 
	*/
	uint8 peer_mac_addr[6];
	/*!< Set peer address to send directed frames to a certain address.
	*/
} tstrM2mAteTx;

/*!
 *@struct	tstrM2mAteRx
 *@brief	Used for the reception(Rx) test configuration.
 */
typedef struct {
	uint8		channel_num;
	/*!< Channel number \ref tenuM2mAteChannels
	 */
	uint8     use_pmu;
	/*!< This is 0 if PMU is not used otherwise it must be be 1 \ref tenuM2mAtePMUSetting
	 */
	uint32     xo_offset_x1000; 
	/*!< Signed XO offset value in PPM (Part Per Million) multiplied by 1000.
	 */
	uint8     use_efuse_xo_offset;
	/*!< Set to 0 to use the XO offset provided in xo_offset_x1000. Set to 1 to use XO offset programmed on WINC efuse. 
	*/
	uint8    self_mac_addr[6];
	/*!< Set to the self mac address required to be overriden. 
	*/
	uint8    peer_mac_addr[6];
	/*!< Set to the source mac address expected to filter frames from. 
	*/
	uint8    mac_filter_en_da;
	/*!< Flag set to enable or disable reception with destination address as a filter. Using the following flags \ref M2M_ATE_RX_ENABLE_DA 
	\ref M2M_ATE_RX_DISABLE_DA 
	*/
	uint8    mac_filter_en_sa;
	/*!< Flag set to enable or disable reception with source address as a filter.Using the following flags \ref M2M_ATE_RX_ENABLE_SA 
	\ref M2M_ATE_RX_DISABLE_SA 
	*/
	uint8   override_self_mac_addr;
	/*!< Flag set to enable or disable self mac address feature. Using the following flags \ref M2M_ATE_DISABLE_SELF_MACADDR 
	\ref M2M_ATE_SET_SELF_MACADDR 
	*/
} tstrM2mAteRx;
  //@}
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION PROTOTYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#ifdef __cplusplus
     extern "C" {
#endif
/**@defgroup  ATEFunction Function
 * @ingroup m2m_ate
 * @{
 */
/*!
@fn	\
	sint8 m2m_ate_init(void);

@brief
	This function used to download the ATE firmware from flash and start it.

@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init_param	
*/
sint8 m2m_ate_init(void);


/*!
@fn	\
	sint8 m2m_ate_init(tstrM2mAteInit *pstrInit);

@brief
	This function is used to download and start the ATE firmware with an initialization value 
	stating the rx mode power \ref tstrM2mAteInit.
@param [in]	tstrM2mAteInit *
	Pointer to a structure \ref tstrM2mAteInit, defining the initial RX mode value.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init
*/
sint8 m2m_ate_init_param(tstrM2mAteInit *pstrInit);

/*!
@fn	\
	sint8 m2m_ate_deinit(void);

@brief
	De-Initialization of ATE firmware mode 

@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
*/
sint8 m2m_ate_deinit(void);

/*!
@fn	\
	sint8 m2m_ate_set_fw_state(uint8);

@brief
	This function is used to change the ATE firmware status from running to stopped or vice versa.

@param [in]	u8State
		Required state of the ATE firmware, one of \ref tenuM2mAteFwState enumeration values.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init
*/
sint8 m2m_ate_set_fw_state(uint8);

/*!
@fn	\
	sint8 m2m_ate_get_fw_state(uint8);

@brief
	This function is used to return the status of ATE firmware.

@return
	The function SHALL return the status of ATE firmware, one of \ref tenuM2mAteFwState enumeration values.
@see
	m2m_ate_init, m2m_ate_set_fw_state
*/
sint8 m2m_ate_get_fw_state(void);

/*!
@fn	\
	uint32 m2m_ate_get_tx_rate(uint8);

@brief
	This function is used to return value of TX rate required by application developer.

@param [in]	u8Index
		Index of the required rate , one of \ref tenuM2mAteTxIndexOfRates enumeration values.
@return
	The function SHALL return 0 in case of receiving invalid index, otherwise the selected rate value is returned.
@see
	tenuM2mAteTxIndexOfRates
*/
uint32 m2m_ate_get_tx_rate(uint8);

/*!
@fn	\
	sint8 m2m_ate_get_tx_status(void);

@brief
	This function is used to return the status of TX test case either running or stopped.

@return
	The function SHALL return the status of ATE firmware, 1 if TX test case is running or 0 if TX test case has been stopped.
@see
	m2m_ate_start_tx, m2m_ate_stop_tx
*/
sint8 m2m_ate_get_tx_status(void);

/*!
@fn	\
	sint8 m2m_ate_start_tx(tstrM2mAteTx *)

@brief
	This function is used to start the TX test case.

@param [in]	strM2mAteTx
		Type of \ref tstrM2mAteTx, with the values required to enable TX test case. Application must use \ref m2m_ate_init first.
@return
	The function SHALL return 0 for success and a negative value otherwise.
@see
	m2m_ate_init, m2m_ate_stop_tx, m2m_ate_get_tx_status
*/
sint8 m2m_ate_start_tx(tstrM2mAteTx *);

/*!
@fn	\
	sint8 m2m_ate_stop_tx(void)

@brief
	This function is used to stop the TX test case.

@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init, m2m_ate_start_tx, m2m_ate_get_tx_status
*/
sint8 m2m_ate_stop_tx(void);

/*!
@fn	\
	sint8 m2m_ate_get_rx_status(uint8);

@brief
	This function is used to return the status of RX test case either running or stopped.

@return
	The function SHALL return status of ATE firmware, 1 if RX test case is running or 0 when the test case has been stopped.
@see
	m2m_ate_start_rx, m2m_ate_stop_rx
*/
sint8 m2m_ate_get_rx_status(void);

/*!
@fn	\
	sint8 m2m_ate_start_rx(tstrM2mAteRx *)

@brief
	This function is used to start RX test case.

@param [in]	strM2mAteRx
		Type of \ref tstrM2mAteRx, with the values required to enable RX test case. Application must use \ref m2m_ate_init first.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init, m2m_ate_stop_rx, m2m_ate_get_rx_status
*/
sint8 m2m_ate_start_rx(tstrM2mAteRx *);

/*!
@fn	\
	sint8 m2m_ate_stop_rx(void)

@brief
	This function is used to stop RX test case.

@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init, m2m_ate_start_rx, m2m_ate_get_rx_status
*/
sint8 m2m_ate_stop_rx(void);

/*!
@fn	\
	sint8 m2m_ate_read_rx_status(tstrM2mAteRxStatus *)

@brief
	This function is used to read RX statistics from the ATE firmware.

@param [out]	strM2mAteRxStatus
		Type of \ref tstrM2mAteRxStatus used to save statistics of RX test case. Application must use \ref m2m_ate_start_rx first.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
	m2m_ate_init, m2m_ate_start_rx
*/
sint8 m2m_ate_read_rx_status(tstrM2mAteRxStatus *);

/*!
@fn	\
	sint8 m2m_ate_set_dig_gain(double dGaindB)

@brief
	This function is used to set the digital gain value to the HW registers in dB.

@param [in]	double dGaindB
		The digital gain value required to be set.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
m2m_ate_get_dig_gain, m2m_ate_get_pa_gain,m2m_ate_get_ppa_gain,m2m_ate_get_tot_gain
*/
sint8 m2m_ate_set_dig_gain(double dGaindB);

/*!
@fn	\
	sint8 m2m_ate_get_dig_gain(double * dGaindB)

@brief
	This function is used to retrieve the digital gain value from the HW registers in dB. 
	Digital gain is one of the values that are set to calculate the total tx gain value.

@param [out]	double * dGaindB
		The retrieved digital gain value obtained from HW registers in dB.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
m2m_ate_set_dig_gain, m2m_ate_get_pa_gain,m2m_ate_get_ppa_gain,m2m_ate_get_tot_gain
*/
sint8 m2m_ate_get_dig_gain(double * dGaindB);

/*!
@fn	\
	void m2m_ate_set_pa_gain(uint8 gain_db)

@brief
	This function is used to set the PA gain (18/15/12/9/6/3/0 only)

@param [in]	uint8 gain_db
		PA gain level allowed (18/15/12/9/6/3/0 only)

*/
void m2m_ate_set_pa_gain(uint8 gain_db);
/*!
@fn	\
	sint8 m2m_ate_get_pa_gain(double *paGaindB)

@brief
	This function is used to get the Power Amplifier(PA) gain

@param [out]	double *paGaindB
	The retrieved PA gain value obtained from HW registers in dB.
@return
		The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
m2m_ate_set_dig_gain, m2m_ate_get_dig_gain,m2m_ate_get_ppa_gain,m2m_ate_get_tot_gain
*/
sint8 m2m_ate_get_pa_gain(double *paGaindB);

/*!
@fn	\
	sint8 m2m_ate_get_ppa_gain(double * ppaGaindB)

@brief
	This function is used to get the Pre-Power Amplifier(PPA) gain

@param [out]	uint32 * ppaGaindB
		The retrieved PPA gain value obtained from HW registers in dB.
@return
	The function SHALL return 0 for success and a negative value otherwise.
@see
m2m_ate_set_dig_gain, m2m_ate_get_dig_gain,m2m_ate_get_pa_gain,m2m_ate_get_tot_gain
*/
sint8 m2m_ate_get_ppa_gain(double * ppaGaindB);

/*!
@fn	\
	sint8 m2m_ate_get_tot_gain(double * totGaindB)

@brief
	This function is used to calculate the total tx gain value

@param [out]	double * totGaindB
		The retrieved total gain value obtained from calculations made based on the digital gain, PA and PPA gain values.
@return
	The function SHALL return @ref M2M_SUCCESS for success and a negative value otherwise.
@see
m2m_ate_set_dig_gain, m2m_ate_get_dig_gain,m2m_ate_get_pa_gain,m2m_ate_get_ppa_gain
*/
sint8 m2m_ate_get_tot_gain(double * totGaindB);
   //@}
	
#ifdef __cplusplus
}
#endif

#endif /* _M2M_CONFIG_MODE_H_ */

#endif //_M2M_ATE_FW_