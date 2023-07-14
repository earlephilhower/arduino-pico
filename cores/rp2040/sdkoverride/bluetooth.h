/*
    Copyright (C) 2015 BlueKitchen GmbH

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
    4. Any redistribution, use, or modification is done solely for
      personal benefit and not for any commercial purpose or for
      monetary gain.

    THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
    ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
    GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Please inquire about commercial licensing options at
    contact@bluekitchen-gmbh.com

*/

/*
    bluetooth.h

    Numbers defined or derived from the official Bluetooth specification
*/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

/**
    @brief hci connection handle type
*/
typedef uint16_t hci_con_handle_t;

/**
    @brief Length of a bluetooth device address.
*/
#define BD_ADDR_LEN 6

/**
    @brief Bluetooth address
*/
typedef uint8_t bd_addr_t[BD_ADDR_LEN];

/**
    Address types
*/
typedef enum {
    // Public Device Address
    BD_ADDR_TYPE_LE_PUBLIC = 0,
    // Random Device Address
    BD_ADDR_TYPE_LE_RANDOM = 1,
    // Public Identity Address (Corresponds to Resolved Private Address)
    BD_ADDR_TYPE_LE_PUBLIC_IDENTITY = 2,
    // Random (static) Identity Address (Corresponds to Resolved Private Address)
    BD_ADDR_TYPE_LE_RANDOM_IDENTITY = 3,
    // internal BTstack addr types for Classic connections
    BD_ADDR_TYPE_SCO       = 0xfc,
    BD_ADDR_TYPE_ACL       = 0xfd,
    BD_ADDR_TYPE_UNKNOWN   = 0xfe,  // also used as 'invalid'
} bd_addr_type_t;

/**
    Link types for BR/EDR Connections
*/
typedef enum {
    HCI_LINK_TYPE_SCO  = 0,
    HCI_LINK_TYPE_ACL  = 1,
    HCI_LINK_TYPE_ESCO = 2,
} hci_link_type_t;


/**
    @brief link key
*/
#define LINK_KEY_LEN 16
#define LINK_KEY_STR_LEN (LINK_KEY_LEN*2)
typedef uint8_t link_key_t[LINK_KEY_LEN];

/**
    @brief link key type
*/
typedef enum {
    INVALID_LINK_KEY = 0xffff,
    COMBINATION_KEY = 0,  // standard pairing
    LOCAL_UNIT_KEY,     // ?
    REMOTE_UNIT_KEY,    // ?
    DEBUG_COMBINATION_KEY,  // SSP with debug
    UNAUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P192, // SSP Simple Pairing
    AUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P192,   // SSP Passkey, Number confirm, OOB
    CHANGED_COMBINATION_KEY,               // Link key changed using Change Connection Lnk Key
    UNAUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P256, // SSP Simpe Pairing
    AUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P256,   // SSP Passkey, Number confirm, OOB
} link_key_type_t;

/**
    LE Privacy 1.2
*/
typedef enum {
    LE_PRIVACY_MODE_NETWORK = 0,
    LE_PRIVACY_MODE_DEVICE = 1,
} le_privacy_mode_t;

/**
    @brief Extended Inquiry Response
*/
#define EXTENDED_INQUIRY_RESPONSE_DATA_LEN 240

/**
    @brief Inquiry modes
*/
typedef enum {
    INQUIRY_MODE_STANDARD = 0,
    INQUIRY_MODE_RSSI,
    INQUIRY_MODE_RSSI_AND_EIR,
} inquiry_mode_t;

/**
    @brief Page Scan Types
*/
typedef enum {
    PAGE_SCAN_MODE_STANDARD = 0,
    PAGE_SCAN_MODE_INTERLACED,
} page_scan_type_t;

/**
    @brief Inquiry Scan Types
*/
typedef enum {
    INQUIRY_SCAN_MODE_STANDARD = 0,
    INQUIRY_SCAN_MODE_INTERLACED,
} inquiry_scan_type_t;

/**
    Link Supervision Timeout Default, 0x7d00 * 0.625ms = 20s
*/
#define HCI_LINK_SUPERVISION_TIMEOUT_DEFAULT 0x7D00

/**
    Service Type used for QoS Setup and Flow Specification
*/
typedef enum {
    HCI_SERVICE_TYPE_NO_TRAFFIC = 0,
    HCI_SERVICE_TYPE_BEST_EFFORT,
    HCI_SERVICE_TYPE_GUARANTEED,
    HCI_SERVICE_TYPE_INVALID,
} hci_service_type_t;

/**
    HCI Transport
*/

/**
    packet types - used in BTstack and over the H4 UART interface
*/
#define HCI_COMMAND_DATA_PACKET 0x01
#define HCI_ACL_DATA_PACKET     0x02
#define HCI_SCO_DATA_PACKET     0x03
#define HCI_EVENT_PACKET        0x04
#define HCI_ISO_DATA_PACKET     0x05

/**
    Other assigned numbers, Assigned_Numbers_Host Controller Interface.pdf
*/

typedef enum {
    HCI_AUDIO_CODING_FORMAT_U_LAW_LOG = 0x00,
    HCI_AUDIO_CODING_FORMAT_A_LAW_LOG,
    HCI_AUDIO_CODING_FORMAT_CVSD,
    HCI_AUDIO_CODING_FORMAT_TRANSPARENT, // Indicates that the controller does not do any transcoding or resampling. This is also used for test mode.
    HCI_AUDIO_CODING_FORMAT_LINEAR_PCM,
    HCI_AUDIO_CODING_FORMAT_MSBC,
    HCI_AUDIO_CODING_FORMAT_LC3,
    HCI_AUDIO_CODING_FORMAT_G_729A,
    HCI_AUDIO_CODING_FORMAT_RFU,
    HCI_AUDIO_CODING_FORMAT_VENDOR_SPECIFIC = 0xFF
} hci_audio_coding_format_t;

/**
    HCI Layer
*/

//
// Error Codes rfom Bluetooth Core Specification
//

/* ENUM_START: BLUETOOTH_ERROR_CODE */
#define ERROR_CODE_SUCCESS                                 0x00
#define ERROR_CODE_UNKNOWN_HCI_COMMAND                     0x01
#define ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER           0x02
#define ERROR_CODE_HARDWARE_FAILURE                        0x03
#define ERROR_CODE_PAGE_TIMEOUT                            0x04
#define ERROR_CODE_AUTHENTICATION_FAILURE                  0x05
#define ERROR_CODE_PIN_OR_KEY_MISSING                      0x06
#define ERROR_CODE_MEMORY_CAPACITY_EXCEEDED                0x07
#define ERROR_CODE_CONNECTION_TIMEOUT                      0x08
#define ERROR_CODE_CONNECTION_LIMIT_EXCEEDED               0x09
#define ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED  0x0A
#define ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS           0x0B
#define ERROR_CODE_COMMAND_DISALLOWED                      0x0C
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES 0x0D
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_SECURITY_REASONS  0x0E
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR 0x0F
#define ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED      0x10
#define ERROR_CODE_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE  0x11
#define ERROR_CODE_INVALID_HCI_COMMAND_PARAMETERS          0x12
#define ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION       0x13
#define ERROR_CODE_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES 0x14
#define ERROR_CODE_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF     0x15
#define ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST     0x16
#define ERROR_CODE_REPEATED_ATTEMPTS                       0x17
#define ERROR_CODE_PAIRING_NOT_ALLOWED                     0x18
#define ERROR_CODE_UNKNOWN_LMP_PDU                         0x19
#define ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE_UNSUPPORTED_LMP_FEATURE 0x1A
#define ERROR_CODE_SCO_OFFSET_REJECTED                     0x1B
#define ERROR_CODE_SCO_INTERVAL_REJECTED                   0x1C
#define ERROR_CODE_SCO_AIR_MODE_REJECTED                   0x1D
#define ERROR_CODE_INVALID_LMP_PARAMETERS_INVALID_LL_PARAMETERS 0x1E
#define ERROR_CODE_UNSPECIFIED_ERROR                       0x1F
#define ERROR_CODE_UNSUPPORTED_LMP_PARAMETER_VALUE_UNSUPPORTED_LL_PARAMETER_VALUE 0x20
#define ERROR_CODE_ROLE_CHANGE_NOT_ALLOWED                 0x21
#define ERROR_CODE_LMP_RESPONSE_TIMEOUT_LL_RESPONSE_TIMEOUT 0x22
#define ERROR_CODE_LMP_ERROR_TRANSACTION_COLLISION         0x23
#define ERROR_CODE_LMP_PDU_NOT_ALLOWED                     0x24
#define ERROR_CODE_ENCRYPTION_MODE_NOT_ACCEPTABLE          0x25
#define ERROR_CODE_LINK_KEY_CANNOT_BE_CHANGED              0x26
#define ERROR_CODE_REQUESTED_QOS_NOT_SUPPORTED             0x27
#define ERROR_CODE_INSTANT_PASSED                          0x28
#define ERROR_CODE_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED     0x29
#define ERROR_CODE_DIFFERENT_TRANSACTION_COLLISION         0x2A
#define ERROR_CODE_RESERVED                                0x2B
#define ERROR_CODE_QOS_UNACCEPTABLE_PARAMETER              0x2C
#define ERROR_CODE_QOS_REJECTED                            0x2D
#define ERROR_CODE_CHANNEL_CLASSIFICATION_NOT_SUPPORTED    0x2E
#define ERROR_CODE_INSUFFICIENT_SECURITY                   0x2F
#define ERROR_CODE_PARAMETER_OUT_OF_MANDATORY_RANGE        0x30
// #define ERROR_CODE_RESERVED
#define ERROR_CODE_ROLE_SWITCH_PENDING                     0x32
// #define ERROR_CODE_RESERVED
#define ERROR_CODE_RESERVED_SLOT_VIOLATION                 0x34
#define ERROR_CODE_ROLE_SWITCH_FAILED                      0x35
#define ERROR_CODE_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE     0x36
#define ERROR_CODE_SECURE_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST 0x37
#define ERROR_CODE_HOST_BUSY_PAIRING                       0x38
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND 0x39
#define ERROR_CODE_CONTROLLER_BUSY                         0x3A
#define ERROR_CODE_UNACCEPTABLE_CONNECTION_PARAMETERS      0x3B
#define ERROR_CODE_DIRECTED_ADVERTISING_TIMEOUT            0x3C
#define ERROR_CODE_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE 0x3D
#define ERROR_CODE_CONNECTION_FAILED_TO_BE_ESTABLISHED     0x3E
#define ERROR_CODE_MAC_CONNECTION_FAILED                   0x3F
#define ERROR_CODE_COARSE_CLOCK_ADJUSTMENT_REJECTED_BUT_WILL_TRY_TO_ADJUST_USING_CLOCK_DRAGGING 0x40

// BTstack defined ERRORS, mapped into BLuetooth status code range

#define BTSTACK_CONNECTION_TO_BTDAEMON_FAILED              0x50
#define BTSTACK_ACTIVATION_FAILED_SYSTEM_BLUETOOTH         0x51
#define BTSTACK_ACTIVATION_POWERON_FAILED                  0x52
#define BTSTACK_ACTIVATION_FAILED_UNKNOWN                  0x53
#define BTSTACK_NOT_ACTIVATED                              0x54
#define BTSTACK_BUSY                                       0x55
#define BTSTACK_MEMORY_ALLOC_FAILED                        0x56
#define BTSTACK_ACL_BUFFERS_FULL                           0x57

// l2cap errors - enumeration by the command that created them
#define L2CAP_COMMAND_REJECT_REASON_COMMAND_NOT_UNDERSTOOD 0x60
#define L2CAP_COMMAND_REJECT_REASON_SIGNALING_MTU_EXCEEDED 0x61
#define L2CAP_COMMAND_REJECT_REASON_INVALID_CID_IN_REQUEST 0x62

#define L2CAP_CONNECTION_RESPONSE_RESULT_SUCCESSFUL        0x63
#define L2CAP_CONNECTION_RESPONSE_RESULT_PENDING           0x64
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_PSM       0x65
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_SECURITY  0x66
#define L2CAP_CONNECTION_RESPONSE_RESULT_REFUSED_RESOURCES 0x67
#define L2CAP_CONNECTION_RESPONSE_RESULT_ERTM_NOT_SUPPORTED 0x68

// should be L2CAP_CONNECTION_RTX_TIMEOUT
#define L2CAP_CONNECTION_RESPONSE_RESULT_RTX_TIMEOUT       0x69
#define L2CAP_CONNECTION_BASEBAND_DISCONNECT               0x6A
#define L2CAP_SERVICE_ALREADY_REGISTERED                   0x6B
#define L2CAP_DATA_LEN_EXCEEDS_REMOTE_MTU                  0x6C
#define L2CAP_SERVICE_DOES_NOT_EXIST                       0x6D
#define L2CAP_LOCAL_CID_DOES_NOT_EXIST                     0x6E
#define L2CAP_CONNECTION_RESPONSE_UNKNOWN_ERROR            0x6F

#define RFCOMM_MULTIPLEXER_STOPPED                         0x70
#define RFCOMM_CHANNEL_ALREADY_REGISTERED                  0x71
#define RFCOMM_NO_OUTGOING_CREDITS                         0x72
#define RFCOMM_AGGREGATE_FLOW_OFF                          0x73
#define RFCOMM_DATA_LEN_EXCEEDS_MTU                        0x74

#define HFP_REMOTE_REJECTS_AUDIO_CONNECTION                0x7F

#define SDP_HANDLE_ALREADY_REGISTERED                      0x80
#define SDP_QUERY_INCOMPLETE                               0x81
#define SDP_SERVICE_NOT_FOUND                              0x82
#define SDP_HANDLE_INVALID                                 0x83
#define SDP_QUERY_BUSY                                     0x84

#define ATT_HANDLE_VALUE_INDICATION_IN_PROGRESS            0x90
#define ATT_HANDLE_VALUE_INDICATION_TIMEOUT                0x91
#define ATT_HANDLE_VALUE_INDICATION_DISCONNECT             0x92

#define GATT_CLIENT_NOT_CONNECTED                          0x93
#define GATT_CLIENT_BUSY                                   0x94
#define GATT_CLIENT_IN_WRONG_STATE                         0x95
#define GATT_CLIENT_DIFFERENT_CONTEXT_FOR_ADDRESS_ALREADY_EXISTS 0x96
#define GATT_CLIENT_VALUE_TOO_LONG                         0x97
#define GATT_CLIENT_CHARACTERISTIC_NOTIFICATION_NOT_SUPPORTED 0x98
#define GATT_CLIENT_CHARACTERISTIC_INDICATION_NOT_SUPPORTED   0x99

#define BNEP_SERVICE_ALREADY_REGISTERED                    0xA0
#define BNEP_CHANNEL_NOT_CONNECTED                         0xA1
#define BNEP_DATA_LEN_EXCEEDS_MTU                          0xA2

// OBEX ERRORS
#define OBEX_UNKNOWN_ERROR                                 0xB0
#define OBEX_CONNECT_FAILED                                0xB1
#define OBEX_DISCONNECTED                                  0xB2
#define OBEX_NOT_FOUND                                     0xB3
#define OBEX_NOT_ACCEPTABLE                                0xB4
#define OBEX_ABORTED                                       0xB5

#define MESH_ERROR_APPKEY_INDEX_INVALID                    0xD0
/* ENUM_END */


/* ENUM_START: AVRCP_BROWSING_ERROR_CODE */
#define AVRCP_BROWSING_ERROR_CODE_INVALID_COMMAND                     0x00  // Sent if TG received a PDU that it did not understand. Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_INVALID_PARAMETER                   0x01  // Sent if the TG received a PDU with a parameter ID that it did not understand. Sent if there is only one parameter ID in the PDU. Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_SPECIFIED_PARAMETER_NOT_FOUND       0x02  // Sent if the parameter ID is understood, but content is wrong or corrupted. Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_INTERNAL_ERROR                      0x03  // Sent if there are error conditions not covered by a more specific error code. Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_SUCCESS                             0x04  // This is the status that should be returned if the operation was successful. Valid for All except where the response CType is AV/C REJECTED.
#define AVRCP_BROWSING_ERROR_CODE_UID_CHANGED                         0x05  // The UIDs on the device have changed. Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_RESERVED_06                         0x06  // Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_INVALID_DIRECTION                   0x07  // The Direction parameter is invalid. Valid for Change Path.
#define AVRCP_BROWSING_ERROR_CODE_NOT_A_DIRECTORY                     0x08  // The UID provided does not refer to a folder item. Valid for Change Path.
#define AVRCP_BROWSING_ERROR_CODE_DOES_NOT_EXIST                      0x09  // The UID provided does not refer to any currently valid. Valid for Change Path, PlayItem, AddToNowPlaying, GetItemAttributes.
#define AVRCP_BROWSING_ERROR_CODE_INVALID_SCOPE                       0x0a  // The scope parameter is invalid. Valid for GetFolderItems, PlayItem, AddToNowPlayer, GetItemAttributes,.
#define AVRCP_BROWSING_ERROR_CODE_RANGE_OUT_OF_BOUNDS                 0x0b  // The start of range provided is not valid. Valid for GetFolderItems.
#define AVRCP_BROWSING_ERROR_CODE_UID_IS_A_DIRECTORY                  0x0c  // The UID provided refers to a directory, which cannot be handled by this media player. Valid for PlayItem, AddToNowPlaying.
#define AVRCP_BROWSING_ERROR_CODE_MEDIA_IN_USES                       0x0d  // The media is not able to be used for this operation at this time. Valid for PlayItem, AddToNowPlaying.
#define AVRCP_BROWSING_ERROR_CODE_NOW_PLAYING_LIST_FULL               0x0e  // No more items can be added to the Now Playing List. Valid for AddToNowPlaying.
#define AVRCP_BROWSING_ERROR_CODE_SEARCH_NOT_SUPPORTED                0x0f  // The Browsed Media Player does not support search. Valid for Search.
#define AVRCP_BROWSING_ERROR_CODE_SEARCH_IN_PROGRESS                  0x10  // A search operation is already in progress. Valid for Search.
#define AVRCP_BROWSING_ERROR_CODE_INVALID_PLAYER_ID                   0x11  // The specified Player Id does not refer to a valid player. Valid for SetAddressedPlayer, SetBrowsedPlayer.
#define AVRCP_BROWSING_ERROR_CODE_PLAYER_NOT_BROWSABLE                0x12  // The Player Id supplied refers to a Media Player which does not support browsing. Valid for SetBrowsedPlayer.
#define AVRCP_BROWSING_ERROR_CODE_PLAYER_NOT_ADDRESSED                0x13  // The Player Id supplied refers to a player which is not currently addressed, and the command is not able to be performed if the player is not set as addressed. Valid for Search SetBrowsedPlayer.
#define AVRCP_BROWSING_ERROR_CODE_NO_VALID_SEARCH_RESULTS             0x14  // The Search result list does not contain valid entries, e.g. after being invalidated due to change of browsed player. Valid for GetFolderItems.
#define AVRCP_BROWSING_ERROR_CODE_NO_AVAILABLE_PLAYERS                0x15  // Valid for All.
#define AVRCP_BROWSING_ERROR_CODE_ADDRESSED_PLAYER_CHANGED            0x16  // Valid for Register Notification.
// 0x17-0xff Reserved
/* ENUM_END */

// HCI roles
typedef enum {
    HCI_ROLE_MASTER = 0,
    HCI_ROLE_SLAVE  = 1,
    HCI_ROLE_INVALID = 0xff,
} hci_role_t;

// packet sizes (max payload)
#define HCI_ACL_DM1_SIZE            17
#define HCI_ACL_DH1_SIZE            27
#define HCI_ACL_2DH1_SIZE           54
#define HCI_ACL_3DH1_SIZE           83
#define HCI_ACL_DM3_SIZE           121
#define HCI_ACL_DH3_SIZE           183
#define HCI_ACL_DM5_SIZE           224
#define HCI_ACL_DH5_SIZE           339
#define HCI_ACL_2DH3_SIZE          367
#define HCI_ACL_3DH3_SIZE          552
#define HCI_ACL_2DH5_SIZE          679
#define HCI_ACL_3DH5_SIZE         1021
#define HCI_SCO_HV1_SIZE            10
#define HCI_SCO_HV2_SIZE            20
#define HCI_SCO_HV3_SIZE            30
#define HCI_SCO_EV3_SIZE            30
#define HCI_SCO_EV4_SIZE           120
#define HCI_SCO_EV5_SIZE           180
#define HCI_SCO_2EV3_SIZE           60
#define HCI_SCO_2EV5_SIZE          360
#define HCI_SCO_3EV3_SIZE           90
#define HCI_SCO_3EV5_SIZE          540

#define LE_ADVERTISING_DATA_SIZE    31
#define LE_EXTENDED_ADVERTISING_DATA_SIZE    229
#define LE_EXTENDED_ADVERTISING_MAX_HANDLE 0xEFu
#define LE_EXTENDED_ADVERTISING_MAX_CHUNK_LEN 251

// advertising event properties for extended advertising
#define LE_ADVERTISING_PROPERTIES_CONNECTABLE      (1u<<0)
#define LE_ADVERTISING_PROPERTIES_SCANNABLE        (1u<<1)
#define LE_ADVERTISING_PROPERTIES_DIRECTED         (1u<<2)
#define LE_ADVERTISING_PROPERTIES_HIGH_DUTY_CYCLE  (1u<<3)
#define LE_ADVERTISING_PROPERTIES_LEGACY           (1u<<4)
#define LE_ADVERTISING_PROPERTIES_ANONYMOUS        (1u<<5)
#define LE_ADVERTISING_PROPERTIES_INCLUDE_TX_POWER (1u<<6)


// SCO Packet Types
#define SCO_PACKET_TYPES_NONE  0x0000
#define SCO_PACKET_TYPES_HV1   0x0001
#define SCO_PACKET_TYPES_HV2   0x0002
#define SCO_PACKET_TYPES_HV3   0x0004
#define SCO_PACKET_TYPES_EV3   0x0008
#define SCO_PACKET_TYPES_EV4   0x0010
#define SCO_PACKET_TYPES_EV5   0x0020
#define SCO_PACKET_TYPES_2EV3  0x0040
#define SCO_PACKET_TYPES_3EV3  0x0080
#define SCO_PACKET_TYPES_2EV5  0x0100
#define SCO_PACKET_TYPES_3EV5  0x0200
#define SCO_PACKET_TYPES_ALL   0x03FF
#define SCO_PACKET_TYPES_SCO   0x0007
#define SCO_PACKET_TYPES_ESCO  0x03F8

// Link Policy Settings
#define LM_LINK_POLICY_DISABLE_ALL_LM_MODES  0
#define LM_LINK_POLICY_ENABLE_ROLE_SWITCH    1
#define LM_LINK_POLICY_ENABLE_HOLD_MODE      2
#define LM_LINK_POLICY_ENABLE_SNIFF_MODE     4

// ACL Connection Modes
#define ACL_CONNECTION_MODE_ACTIVE 0
#define ACL_CONNECTION_MODE_HOLD   1
#define ACL_CONNECTION_MODE_SNIFF  2

/**
    Default INQ Mode
*/
#define GAP_IAC_GENERAL_INQUIRY 0x9E8B33L // General/Unlimited Inquiry Access Code (GIAC)
#define GAP_IAC_LIMITED_INQUIRY 0x9E8B00L // Limited Dedicated Inquiry Access Code (LIAC)

/**
    SSP IO Capabilities
*/
#define SSP_IO_CAPABILITY_DISPLAY_ONLY   0
#define SSP_IO_CAPABILITY_DISPLAY_YES_NO 1
#define SSP_IO_CAPABILITY_KEYBOARD_ONLY  2
#define SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT 3
#define SSP_IO_CAPABILITY_UNKNOWN 0xff


/**
    SSP Authentication Requirements, see IO Capability Request Reply Command
*/

// Numeric comparison with automatic accept allowed.
#define SSP_IO_AUTHREQ_MITM_PROTECTION_NOT_REQUIRED_NO_BONDING 0x00

// Use IO Capabilities to deter- mine authentication procedure
#define SSP_IO_AUTHREQ_MITM_PROTECTION_REQUIRED_NO_BONDING 0x01

// Numeric compar- ison with automatic accept allowed.
#define SSP_IO_AUTHREQ_MITM_PROTECTION_NOT_REQUIRED_DEDICATED_BONDING 0x02

// Use IO Capabilities to determine authentication procedure
#define SSP_IO_AUTHREQ_MITM_PROTECTION_REQUIRED_DEDICATED_BONDING 0x03

// Numeric Compari- son with automatic accept allowed.
#define SSP_IO_AUTHREQ_MITM_PROTECTION_NOT_REQUIRED_GENERAL_BONDING 0x04

// Use IO capabilities to determine authentication procedure.
#define SSP_IO_AUTHREQ_MITM_PROTECTION_REQUIRED_GENERAL_BONDING 0x05


// OGFs
#define OGF_LINK_CONTROL          0x01
#define OGF_LINK_POLICY           0x02
#define OGF_CONTROLLER_BASEBAND   0x03
#define OGF_INFORMATIONAL_PARAMETERS 0x04
#define OGF_STATUS_PARAMETERS     0x05
#define OGF_TESTING               0x06
#define OGF_LE_CONTROLLER 0x08
#define OGF_VENDOR  0x3f




/**
    L2CAP Layer
*/

#define L2CAP_HEADER_SIZE 4

// minimum signaling MTU
#define L2CAP_MINIMAL_MTU 48
#define L2CAP_DEFAULT_MTU 672

// Minimum/default MTU
#define L2CAP_LE_DEFAULT_MTU  23

// L2CAP Fixed Channel IDs
#define L2CAP_CID_SIGNALING                        0x0001
#define L2CAP_CID_CONNECTIONLESS_CHANNEL           0x0002
#define L2CAP_CID_ATTRIBUTE_PROTOCOL               0x0004
#define L2CAP_CID_SIGNALING_LE                     0x0005
#define L2CAP_CID_SECURITY_MANAGER_PROTOCOL        0x0006
#define L2CAP_CID_BR_EDR_SECURITY_MANAGER          0x0007

// L2CAP Channels in Basic and Enhanced Retransmission Mode

// connection response result
#define L2CAP_CONNECTION_RESULT_SUCCESS                         0x0000
#define L2CAP_CONNECTION_RESULT_PENDING                         0x0001
#define L2CAP_CONNECTION_RESULT_PSM_NOT_SUPPORTED               0x0002
#define L2CAP_CONNECTION_RESULT_SECURITY_BLOCK                  0x0003
#define L2CAP_CONNECTION_RESULT_NO_RESOURCES_AVAILABLE          0x0004
#define L2CAP_CONNECTION_RESULT_INVALID_SOURCE_CID              0x0006
#define L2CAP_CONNECTION_RESULT_SOURCE_CID_ALREADY_ALLOCATED    0x0007

// L2CAP Channels in LE Credit-Based Flow-Control Mode

// connection response result
#define L2CAP_CBM_CONNECTION_RESULT_SUCCESS                         0x0000
#define L2CAP_CBM_CONNECTION_RESULT_SPSM_NOT_SUPPORTED              0x0002
#define L2CAP_CBM_CONNECTION_RESULT_NO_RESOURCES_AVAILABLE          0x0004
#define L2CAP_CBM_CONNECTION_RESULT_INSUFFICIENT_AUTHENTICATION     0x0005
#define L2CAP_CBM_CONNECTION_RESULT_INSUFFICIENT_AUTHORIZATION      0x0006
#define L2CAP_CBM_CONNECTION_RESULT_ENCYRPTION_KEY_SIZE_TOO_SHORT   0x0007
#define L2CAP_CBM_CONNECTION_RESULT_INSUFFICIENT_ENCRYPTION         0x0008
#define L2CAP_CBM_CONNECTION_RESULT_INVALID_SOURCE_CID              0x0009
#define L2CAP_CBM_CONNECTION_RESULT_SOURCE_CID_ALREADY_ALLOCATED    0x000A
#define L2CAP_CBM_CONNECTION_RESULT_UNACCEPTABLE_PARAMETERS         0x000B


// L2CAP Channels in Enhanced Credit-Based Flow-Control Mode

// number of CIDs in single connection+reconfiguration request/response
#define L2CAP_ECBM_MAX_CID_ARRAY_SIZE        5

// connection response result
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_SUCCESS                                    0x0000
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_SPSM_NOT_SUPPORTED                 0x0002
#define L2CAP_ECBM_CONNECTION_RESULT_SOME_REFUSED_INSUFFICIENT_RESOURCES_AVAILABLE  0x0004
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_INSUFFICIENT_AUTHENTICATION        0x0005
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_INSUFFICIENT_AUTHORIZATION         0x0006
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_ENCYRPTION_KEY_SIZE_TOO_SHORT      0x0007
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_INSUFFICIENT_ENCRYPTION            0x0008
#define L2CAP_ECBM_CONNECTION_RESULT_SOME_REFUSED_INVALID_SOURCE_CID                0x0009
#define L2CAP_ECBM_CONNECTION_RESULT_SOME_REFUSED_SOURCE_CID_ALREADY_ALOCATED       0x000A
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_UNACCEPTABLE_PARAMETERS            0x000B
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_REFUSED_INVALID_PARAMETERS                 0x000C
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_PENDING_NO_FURTHER_INFORMATION             0x000D
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_PENDING_AUTHENTICATION                     0x000E
#define L2CAP_ECBM_CONNECTION_RESULT_ALL_PENDING_AUTHORIZATION                      0x000F


// Result for Reconfigure Request
#define L2CAP_ECBM_RECONFIGURE_SUCCESS                                 0
#define L2CAP_ECBM_RECONFIGURE_FAILED_MTU_REDUCTION_NOT_ALLOWED        1
#define L2CAP_ECBM_RECONFIGURE_FAILED_MPS_REDUCTION_MULTIPLE_CHANNELS  2
#define L2CAP_ECBM_RECONFIGURE_FAILED_DESTINATION_CID_INVALID          3
#define L2CAP_ECBM_RECONFIGURE_FAILED_UNACCEPTABLE_PARAMETERS          4

/**
    SDP Protocol
*/

// Device Vendor ID Sources
#define DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH 0x0001
#define DEVICE_ID_VENDOR_ID_SOURCE_USB       0x0002

// OBEX
#define SDP_vCard_2_1       0x01
#define SDP_vCard_3_0       0x02
#define SDP_vCal_1_0        0x03
#define SDP_iCal_2_0        0x04
#define SDP_vNote           0x05
#define SDP_vMessage        0x06
#define SDP_OBEXFileTypeAny 0xFF

/**
    RFCOMM Protocol
*/

// Line Status
#define LINE_STATUS_NO_ERROR       0x00
#define LINE_STATUS_OVERRUN_ERROR  0x03
#define LINE_STATUS_PARITY_ERORR   0x05
#define LINE_STATUS_FRAMING_ERROR  0x09

// Modem Status Flags
#define MODEM_STATUS_FC   0x02
#define MODEM_STATUS_RTC  0x04
#define MODEM_STATUS_RTR  0x08
#define MODEM_STATUS_IC   0x40
#define MODEM_STATUS_DV   0x80

typedef enum rpn_baud {
    RPN_BAUD_2400 = 0,
    RPN_BAUD_4800,
    RPN_BAUD_7200,
    RPN_BAUD_9600,
    RPN_BAUD_19200,
    RPN_BAUD_38400,
    RPN_BAUD_57600,
    RPN_BAUD_115200,
    RPN_BAUD_230400
} rpn_baud_t;

typedef enum rpn_data_bits {
    RPN_DATA_BITS_5 = 0,
    RPN_DATA_BITS_6 = 0,
    RPN_DATA_BITS_7 = 0,
    RPN_DATA_BITS_8 = 0
} rpn_data_bits_t;

typedef enum rpn_stop_bits {
    RPN_STOP_BITS_1_0 = 0,
    RPN_STOP_BITS_1_5
} rpn_stop_bits_t;

typedef enum rpn_parity {
    RPN_PARITY_NONE  = 0,
    RPN_PARITY_ODD   = 1,
    RPN_PARITY_EVEN  = 3,
    RPN_PARITY_MARK  = 5,
    RPN_PARITY_SPACE = 7,
} rpn_parity_t;

#define RPN_FLOW_CONTROL_XONXOFF_ON_INPUT  0x01
#define RPN_FLOW_CONTROL_XONXOFF_ON_OUTPUT 0x02
#define RPN_FLOW_CONTROL_RTR_ON_INPUT      0x04
#define RPN_FLOW_CONTROL_RTR_ON_OUTPUT     0x08
#define RPN_FLOW_CONTROL_RTC_ON_INPUT      0x10
#define RPN_FLOW_CONTROL_RTC_ON_OUTPUT     0x20

#define RPN_PARAM_MASK_0_BAUD             0x01
#define RPN_PARAM_MASK_0_DATA_BITS        0x02
#define RPN_PARAM_MASK_0_STOP_BITS        0x04
#define RPN_PARAM_MASK_0_PARITY           0x08
#define RPN_PARAM_MASK_0_PARITY_TYPE      0x10
#define RPN_PARAM_MASK_0_XON_CHAR         0x20
#define RPN_PARAM_MASK_0_XOFF_CHAR        0x40
#define RPN_PARAM_MASK_0_RESERVED         0x80

// @note: values are identical to rpn_flow_control_t
#define RPN_PARAM_MASK_1_XONOFF_ON_INPUT  0x01
#define RPN_PARAM_MASK_1_XONOFF_ON_OUTPUT 0x02
#define RPN_PARAM_MASK_1_RTR_ON_INPUT     0x04
#define RPN_PARAM_MASK_1_RTR_ON_OUTPUT    0x08
#define RPN_PARAM_MASK_1_RTC_ON_INPUT     0x10
#define RPN_PARAM_MASK_1_RTC_ON_OUTPUT    0x20
#define RPN_PARAM_MASK_1_RESERVED_0       0x40
#define RPN_PARAM_MASK_1_RESERVED_1       0x80

/**
    BNEP Protocol
*/

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#ifndef ETHERTYPE_VLAN
#define ETHERTYPE_VLAN                                  0x8100 /* IEEE 802.1Q VLAN tag */
#endif

#define BNEP_MTU_MIN                                    1691


/**
    PAN Profile
*/

typedef enum {
    BNEP_SECURITY_NONE = 0x0000,
    BNEP_SECURITY_SERVICE_LEVEL_ENFORCED,
    BNEP_SECURITY_802_1X
} security_description_t;

typedef enum {
    PAN_NET_ACCESS_TYPE_PSTN = 0x0000,
    PAN_NET_ACCESS_TYPE_ISDN,
    PAN_NET_ACCESS_TYPE_DSL,
    PAN_NET_ACCESS_TYPE_CABLE_MODEM,
    PAN_NET_ACCESS_TYPE_10MB_ETHERNET,
    PAN_NET_ACCESS_TYPE_100MB_ETHERNET,
    PAN_NET_ACCESS_TYPE_4MB_TOKEN_RING,
    PAN_NET_ACCESS_TYPE_16MB_TOKEN_RING,
    PAN_NET_ACCESS_TYPE_100MB_TOKEN_RING,
    PAN_NET_ACCESS_TYPE_FDDI,
    PAN_NET_ACCESS_TYPE_GSM,
    PAN_NET_ACCESS_TYPE_CDMA,
    PAN_NET_ACCESS_TYPE_GPRS,
    PAN_NET_ACCESS_TYPE_3G,
    PAN_NET_ACCESS_TYPE_CELULAR,
    PAN_NET_ACCESS_TYPE_OTHER = 0xFFFE,
    PAN_NET_ACCESS_TYPE_NONE
} net_access_type_t;

/**
    ATT
*/

// Minimum/default MTU
#define ATT_DEFAULT_MTU               23

// MARK: ATT Error Codes
#define ATT_ERROR_SUCCESS                          0x00
#define ATT_ERROR_INVALID_HANDLE                   0x01
#define ATT_ERROR_READ_NOT_PERMITTED               0x02
#define ATT_ERROR_WRITE_NOT_PERMITTED              0x03
#define ATT_ERROR_INVALID_PDU                      0x04
#define ATT_ERROR_INSUFFICIENT_AUTHENTICATION      0x05
#define ATT_ERROR_REQUEST_NOT_SUPPORTED            0x06
#define ATT_ERROR_INVALID_OFFSET                   0x07
#define ATT_ERROR_INSUFFICIENT_AUTHORIZATION       0x08
#define ATT_ERROR_PREPARE_QUEUE_FULL               0x09
#define ATT_ERROR_ATTRIBUTE_NOT_FOUND              0x0a
#define ATT_ERROR_ATTRIBUTE_NOT_LONG               0x0b
#define ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE 0x0c
#define ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH   0x0d
#define ATT_ERROR_UNLIKELY_ERROR                   0x0e
#define ATT_ERROR_INSUFFICIENT_ENCRYPTION          0x0f
#define ATT_ERROR_UNSUPPORTED_GROUP_TYPE           0x10
#define ATT_ERROR_INSUFFICIENT_RESOURCES           0x11
#define ATT_ERROR_VALUE_NOT_ALLOWED                0x13

// MARK: ATT Error Codes defined by BTstack
#define ATT_ERROR_HCI_DISCONNECT_RECEIVED          0x1f
#define ATT_ERROR_BONDING_INFORMATION_MISSING      0x70
#define ATT_ERROR_DATA_MISMATCH                    0x7e
#define ATT_ERROR_TIMEOUT                          0x7F
#define ATT_ERROR_WRITE_RESPONSE_PENDING           0x100

// MARK: ATT Error Codes from Bluetooth Core Specification Supplement, Version 9 or later
#define ATT_ERROR_WRITE_REQUEST_REJECTED                                                      0xFC
#define ATT_ERROR_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_IMPROPERLY_CONFIGURED        0xFD
#define ATT_ERROR_PROCEDURE_ALREADY_IN_PROGRESS                                               0xFE
#define ATT_ERROR_OUT_OF_RANGE                                                                0xFF

// MARK: ATT Error Codes from Cycling Power Service spec
#define CYCLING_POWER_ERROR_CODE_INAPPROPRIATE_CONNECTION_PARAMETERS                          0x80
#define CYCLING_POWER_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS                                0xFE
#define CYCLING_POWER_ERROR_CODE_CCC_DESCRIPTOR_IMPROPERLY_CONFIGURED                         0xFD

// MARK: ATT Error Codes from Cycling Speed and Cadence Service spec
#define CYCLING_SPEED_AND_CADENCE_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS                    0x80
#define CYCLING_SPEED_AND_CADENCE_ERROR_CODE_CCC_DESCRIPTOR_IMPROPERLY_CONFIGURED             0x81


// MARK: Attribute Property Flags
#define ATT_PROPERTY_BROADCAST           0x01
#define ATT_PROPERTY_READ                0x02
#define ATT_PROPERTY_WRITE_WITHOUT_RESPONSE 0x04
#define ATT_PROPERTY_WRITE               0x08
#define ATT_PROPERTY_NOTIFY              0x10
#define ATT_PROPERTY_INDICATE            0x20
#define ATT_PROPERTY_AUTHENTICATED_SIGNED_WRITE 0x40
#define ATT_PROPERTY_EXTENDED_PROPERTIES 0x80

// MARK: Attribute Property Flag, BTstack extension
// value is asked from client
#define ATT_PROPERTY_DYNAMIC             0x100

// Security levels
#define ATT_SECURITY_NONE 0
#define ATT_SECURITY_ENCRYPTED 1
#define ATT_SECURITY_AUTHENTICATED 2
#define ATT_SECURITY_AUTHORIZED 3
#define ATT_SECURITY_AUTHENTICATED_SC 4

// ATT Transaction Timeout of 30 seconds for Command/Response or Indication/Confirmation
#define ATT_TRANSACTION_TIMEOUT_MS     30000

#define ATT_TRANSACTION_MODE_NONE      0x0
#define ATT_TRANSACTION_MODE_ACTIVE    0x1
#define ATT_TRANSACTION_MODE_EXECUTE   0x2
#define ATT_TRANSACTION_MODE_CANCEL    0x3
#define ATT_TRANSACTION_MODE_VALIDATE  0x4

// MARK: GATT UUIDs
#define GATT_PRIMARY_SERVICE_UUID                   0x2800
#define GATT_SECONDARY_SERVICE_UUID                 0x2801
#define GATT_INCLUDE_SERVICE_UUID                   0x2802
#define GATT_CHARACTERISTICS_UUID                   0x2803
#define GATT_CHARACTERISTIC_EXTENDED_PROPERTIES     0x2900
#define GATT_CHARACTERISTIC_USER_DESCRIPTION        0x2901
#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION   0x2902
#define GATT_SERVER_CHARACTERISTICS_CONFIGURATION   0x2903
#define GATT_CHARACTERISTIC_PRESENTATION_FORMAT     0x2904
#define GATT_CHARACTERISTIC_AGGREGATE_FORMAT        0x2905
#define GATT_CLIENT_SUPPORTED_FEATURES              0x2B29
#define GATT_SERVER_SUPPORTED_FEATURES              0x2B3A

#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE          0
#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION  1
#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION    2

#define GATT_CLIENT_ANY_CONNECTION      0xffff
#define GATT_CLIENT_ANY_VALUE_HANDLE    0x0000

// GAP Service and Characteristics
#define GAP_SERVICE_UUID               0x1800
#define GAP_DEVICE_NAME_UUID           0x2a00
#define GAP_APPEARANCE_UUID            0x2a01
#define GAP_PERIPHERAL_PRIVACY_FLAG    0x2a02
#define GAP_RECONNECTION_ADDRESS_UUID  0x2a03
#define GAP_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS_UUID 0x2a04
#define GAP_SERVICE_CHANGED            0x2a05

// Bluetooth GATT types

typedef struct {
    uint16_t year;      // 0 - year  is not known; or [1582,9999]
    uint8_t  month;     // 0 - month is not known; or [1,12]
    uint8_t  day;       // 0 - day   is not known; or [1,31]
    uint8_t  hours;     // [0,23]
    uint8_t  minutes;   // [0,59]
    uint8_t  seconds;   // [0,59]
} gatt_date_time_t;

typedef enum {
    GATT_MICROPHONE_CONTROL_MUTE_OFF = 0x00,
    GATT_MICROPHONE_CONTROL_MUTE_ON,
    GATT_MICROPHONE_CONTROL_MUTE_DISABLED
} gatt_microphone_control_mute_t;

/**
    SM - LE Security Manager
*/
// Bluetooth Spec definitions
typedef enum {
    SM_CODE_PAIRING_REQUEST = 0X01,
    SM_CODE_PAIRING_RESPONSE,
    SM_CODE_PAIRING_CONFIRM,
    SM_CODE_PAIRING_RANDOM,
    SM_CODE_PAIRING_FAILED,
    SM_CODE_ENCRYPTION_INFORMATION,
    SM_CODE_MASTER_IDENTIFICATION,
    SM_CODE_IDENTITY_INFORMATION,
    SM_CODE_IDENTITY_ADDRESS_INFORMATION,
    SM_CODE_SIGNING_INFORMATION,
    SM_CODE_SECURITY_REQUEST,
    SM_CODE_PAIRING_PUBLIC_KEY,
    SM_CODE_PAIRING_DHKEY_CHECK,
    SM_CODE_KEYPRESS_NOTIFICATION,
} SECURITY_MANAGER_COMMANDS;

// IO Capability Values
typedef enum {
    IO_CAPABILITY_DISPLAY_ONLY = 0,
    IO_CAPABILITY_DISPLAY_YES_NO,
    IO_CAPABILITY_KEYBOARD_ONLY,
    IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
    IO_CAPABILITY_KEYBOARD_DISPLAY, // not used by secure simple pairing
} io_capability_t;

// Authentication requirement flags
#define SM_AUTHREQ_NO_BONDING        0x00
#define SM_AUTHREQ_BONDING           0x01
#define SM_AUTHREQ_MITM_PROTECTION   0x04
#define SM_AUTHREQ_SECURE_CONNECTION 0x08
#define SM_AUTHREQ_KEYPRESS          0x10
#define SM_AUTHREQ_CT2               0x20

// Key distribution flags used by spec
#define SM_KEYDIST_ENC_KEY  0x01
#define SM_KEYDIST_ID_KEY   0x02
#define SM_KEYDIST_SIGN     0x04
#define SM_KEYDIST_LINK_KEY 0x08

// Key distribution flags used internally
#define SM_KEYDIST_FLAG_ENCRYPTION_INFORMATION       0x01
#define SM_KEYDIST_FLAG_MASTER_IDENTIFICATION        0x02
#define SM_KEYDIST_FLAG_IDENTITY_INFORMATION         0x04
#define SM_KEYDIST_FLAG_IDENTITY_ADDRESS_INFORMATION 0x08
#define SM_KEYDIST_FLAG_SIGNING_IDENTIFICATION       0x10

// STK Generation Methods
#define SM_STK_GENERATION_METHOD_JUST_WORKS          0x01
#define SM_STK_GENERATION_METHOD_OOB                 0x02
#define SM_STK_GENERATION_METHOD_PASSKEY             0x04
#define SM_STK_GENERATION_METHOD_NUMERIC_COMPARISON  0x08

// Pairing Failed Reasons
#define SM_REASON_RESERVED                     0x00
#define SM_REASON_PASSKEY_ENTRY_FAILED         0x01
#define SM_REASON_OOB_NOT_AVAILABLE            0x02
#define SM_REASON_AUTHENTHICATION_REQUIREMENTS 0x03
#define SM_REASON_CONFIRM_VALUE_FAILED         0x04
#define SM_REASON_PAIRING_NOT_SUPPORTED        0x05
#define SM_REASON_ENCRYPTION_KEY_SIZE          0x06
#define SM_REASON_COMMAND_NOT_SUPPORTED        0x07
#define SM_REASON_UNSPECIFIED_REASON           0x08
#define SM_REASON_REPEATED_ATTEMPTS            0x09
#define SM_REASON_INVALID_PARAMETERS           0x0a
#define SM_REASON_DHKEY_CHECK_FAILED           0x0b
#define SM_REASON_NUMERIC_COMPARISON_FAILED    0x0c
#define SM_REASON_BR_EDR_PAIRING_IN_PROGRESS   0x0d
#define SM_REASON_CROSS_TRANSPORT_KEY_DERIVATION_NOT_ALLOWED 0x0e
#define SM_REASON_KEY_REJECTED                 0x0f

// also, invalid parameters
// and reserved

// Keypress Notifications
#define SM_KEYPRESS_PASSKEY_ENTRY_STARTED      0x00
#define SM_KEYPRESS_PASSKEY_DIGIT_ENTERED      0x01
#define SM_KEYPRESS_PASSKEY_DIGIT_ERASED       0x02
#define SM_KEYPRESS_PASSKEY_CLEARED            0x03
#define SM_KEYPRESS_PASSKEY_ENTRY_COMPLETED    0x04


#endif
