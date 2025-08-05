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

/** @file KeyRange.c
 *
 * Implementation of the KeyRange class.
 */

#include "KeyRange.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "Key.h"

ErrorCode KeyRange_write(const KeyRange* this, BufferedOStream* output)
{
	ErrorCode res = Key_write(this->min, output);
	if (res != ERR_OK) {
		return res;
	}
	return Key_write(this->max, output);
}
