#pragma once

#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

#include "webcface/encoding/encoding.h"
#include "webcface/encoding/val_adaptor.h"

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

// 〜v1.1 との互換性のため
namespace WebCFace = webcface;
