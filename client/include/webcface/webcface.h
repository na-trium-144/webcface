#pragma once
#ifdef WEBCFACE_STATIC_DIR
// clang-format off
#error "This header file is for WebCFace ver.1.x but is used with WebCFace ver.0.x CMake files"
// clang-format on
#endif

// ヘッダーの上下関係はここに書かれた順

#include <webcface/common/def.h>
#include "wcf.h"

#include "encoding.h"
#include "common/version.h"
#include "val_adaptor.h"
#include "common/func.h"
#include "common/queue.h"
#include "common/log.h"
#include "common/view.h"
#include "common/dict.h"
#include "common/image.h"
#include "common/transform.h"
#include "common/canvas3d.h"
#include "common/canvas2d.h"
#include "common/robot_model.h"

#include "field.h"
#include "event_target.h"

#include "func_result.h"
#include "logger.h"

#include "value.h"
#include "text.h"
#include "log.h"
#include "func.h"
#include "components.h"
#include "view.h"
#include "image.h"
#include "robot_model.h"
#include "canvas3d.h"
#include "canvas2d.h"

#include "member.h"
#include "client.h"

#include "server.h"

// 〜v1.1 との互換性のため
namespace WebCFace = webcface;