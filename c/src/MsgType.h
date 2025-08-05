/*
 * Copyright 2025 Atende Industries sp. z o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/** @file MsgType.h
 *
 * TStorage request message types.
 */

#ifndef MSGTYPE_H
#define MSGTYPE_H

/**
 * @brief TStorage request message types.
 */
typedef enum MsgType {
	MSG_NULL = 0,
	MSG_GET = 1,
	MSG_PUTSAFE = 5,
	MSG_PUTASAFE = 6,
	MSG_GETACQ = 7
} MsgType;

#endif /* MSGTYPE_H */