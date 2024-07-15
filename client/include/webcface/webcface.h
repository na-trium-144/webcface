#pragma once
#ifdef WEBCFACE_STATIC_DIR
// clang-format off
#error "This header file is for WebCFace ver.1.x but is used with WebCFace ver.0.x CMake files"
// clang-format on
#endif

#include "webcface/common/def.h"

#include "encoding/encoding.h"
#include "encoding/val_adaptor.h"

#include "wcf.h"

#include "version.h"
#include "func_info.h"
#include "components.h"
#include "image_frame.h"
#include "transform.h"
#include "robot_link.h"
#include "field.h"
#include "func_result.h"

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
