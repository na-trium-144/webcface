#pragma once
// ヘッダーの上下関係はここに書かれた順

#include "common/def.h"
#include "common/version.h"
#include "common/val.h"
#include "common/func.h"
#include "common/queue.h"
#include "common/field_base.h"
#include "common/log.h"
#include "common/view.h"
#include "common/dict.h"

#include "field.h"
#include "event_target.h"

#include "member.h"
#include "func_result.h"
#include "logger.h"

#include "client_data.h"

// ClientDataに対して具体的になにかする場合はこれの下
#include "value.h"
#include "text.h"
#include "log.h"
#include "func.h"
#include "view.h"

#include "client.h"

namespace webcface = WebCFace;
