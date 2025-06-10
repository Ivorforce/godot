/**************************************************************************/
/*  thread_safe.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef THREAD_SAFE_CPP
#define THREAD_SAFE_CPP

#include "thread_safe.h"

#include "core/string/print_string.h"

#include <thread>

inline std::thread::id current_node_thread{};

bool is_current_thread_safe_for_nodes() {
	return std::this_thread::get_id() == current_node_thread;
}

void set_current_thread_safe_for_nodes(bool p_safe) {
	const std::thread::id thread_id = std::this_thread::get_id();

	if (p_safe) {
		if (current_node_thread == thread_id) {
			return; // Nothing to do.
		}

		if (current_node_thread != std::thread::id{}) {
			print_error("Updating the main node thread when another was already registered.");
		}

		current_node_thread = thread_id;
	}
	else if (current_node_thread == thread_id) {
		// Reset the ID.
		current_node_thread = std::thread::id{};
	}
}

#endif // THREAD_SAFE_CPP
