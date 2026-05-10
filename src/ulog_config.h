
#pragma once

/******************************************************************************
 ULOG_BUILD_COLOR                   0                           Compile color code paths
 ULOG_BUILD_PREFIX_SIZE             0                           Prefix buffer logic
 ULOG_BUILD_EXTRA_OUTPUTS           0                           Extra output backends
 ULOG_BUILD_SOURCE_LOCATION         1                           File:line output
 ULOG_BUILD_LEVEL_SHORT             0                           Print levels with short names, e.g. 'E'
 ULOG_BUILD_TIME                    0                           Timestamp support
 ULOG_BUILD_TOPICS_MODE             ULOG_BUILD_TOPICS_MODE_OFF  Topic allocation mode
 ULOG_BUILD_TOPICS_STATIC_NUM       0                           Number of static topics (0 = disabled)
 ULOG_BUILD_DYNAMIC_CONFIG          0                           Runtime toggles
 ULOG_BUILD_WARN_NOT_ENABLED        1                           Warning stubs
 ULOG_BUILD_CONFIG_HEADER_ENABLED   0                           Use external configuration header
 ULOG_BUILD_CONFIG_HEADER_NAME      "ulog_config.h"             Configuration header name
 ULOG_BUILD_DISABLED                0                           Disable microlog completely
*******************************************************************************/

// Define all build options in one place
#define ULOG_BUILD_COLOR                1
#define ULOG_BUILD_PREFIX_SIZE          0
#define ULOG_BUILD_EXTRA_OUTPUTS        1
#define ULOG_BUILD_SOURCE_LOCATION      1
#define ULOG_BUILD_LEVEL_SHORT          0
#define ULOG_BUILD_TIME                 0
#define ULOG_BUILD_TOPICS_MODE          ULOG_BUILD_TOPICS_MODE_STATIC
#define ULOG_BUILD_TOPICS_STATIC_NUM    8
#define ULOG_BUILD_DYNAMIC_CONFIG       1
#define ULOG_BUILD_WARN_NOT_ENABLED     1
