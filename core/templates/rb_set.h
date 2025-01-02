/**************************************************************************/
/*  rb_set.h                                                              */
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

#ifndef RB_SET_H
#define RB_SET_H

#include "core/templates/rb_tree.h"

// based on the very nice implementation of rb-trees by:
// https://web.archive.org/web/20120507164830/https://web.mit.edu/~emin/www/source_code/red_black_tree/index.html

template <typename T, typename C = Comparator<T>, typename A = DefaultAllocator>
class RBSet : public RBTree<RBElement<T, A>, A> {
public:
	using Element = RBElement<T, A>;
	typedef T ValueType;
private:
	using Super = RBTree<Element, A>;
	using Color = typename Element::Color;
	using _Data = typename Super::_Data;

	Element *_find(const T &p_value) const {
		Element *node = Super::_data._root->left;
		C less;

		while (node != Super::_data._nil) {
			if (less(p_value, node->value)) {
				node = node->left;
			} else if (less(node->value, p_value)) {
				node = node->right;
			} else {
				return node; // found
			}
		}

		return nullptr;
	}

	Element *_lower_bound(const T &p_value) const {
		Element *node = Super::_data._root->left;
		Element *prev = nullptr;
		C less;

		while (node != Super::_data._nil) {
			prev = node;

			if (less(p_value, node->value)) {
				node = node->left;
			} else if (less(node->value, p_value)) {
				node = node->right;
			} else {
				return node; // found
			}
		}

		if (prev == nullptr) {
			return nullptr; // tree empty
		}

		if (less(prev->value, p_value)) {
			prev = prev->_next;
		}

		return prev;
	}

	Element *_insert(const T &p_value) {
		Element *new_parent = Super::_data._root;
		Element *node = Super::_data._root->left;
		C less;

		while (node != Super::_data._nil) {
			new_parent = node;

			if (less(p_value, node->value)) {
				node = node->left;
			} else if (less(node->value, p_value)) {
				node = node->right;
			} else {
				return node; // Return existing node
			}
		}

		Element *new_node = memnew_allocator(Element, A);
		new_node->parent = new_parent;
		new_node->right = Super::_data._nil;
		new_node->left = Super::_data._nil;
		new_node->value = p_value;
		//new_node->data=_data;

		if (new_parent == Super::_data._root || less(p_value, new_parent->value)) {
			new_parent->left = new_node;
		} else {
			new_parent->right = new_node;
		}

		new_node->_next = Super::_successor(new_node);
		new_node->_prev = Super::_predecessor(new_node);
		if (new_node->_next) {
			new_node->_next->_prev = new_node;
		}
		if (new_node->_prev) {
			new_node->_prev->_next = new_node;
		}

		Super::_data.size_cache++;
		_insert_rb_fix(new_node);
		return new_node;
	}

	void _copy_from(const RBSet &p_set) {
		Super::clear();
		// not the fastest way, but safeset to write.
		for (Element *I = p_set.front(); I; I = I->next()) {
			insert(I->get());
		}
	}

public:
	const Element *find(const T &p_value) const {
		if (!Super::_data._root) {
			return nullptr;
		}

		const Element *res = _find(p_value);
		return res;
	}

	Element *find(const T &p_value) {
		if (!Super::_data._root) {
			return nullptr;
		}

		Element *res = _find(p_value);
		return res;
	}

	Element *lower_bound(const T &p_value) const {
		if (!Super::_data._root) {
			return nullptr;
		}
		return _lower_bound(p_value);
	}

	bool has(const T &p_value) const {
		return find(p_value) != nullptr;
	}

	Element *insert(const T &p_value) {
		if (!Super::_data._root) {
			Super::_data._create_root();
		}
		return _insert(p_value);
	}

	bool erase(const T &p_value) {
		if (!Super::_data._root) {
			return false;
		}

		Element *e = find(p_value);
		if (!e) {
			return false;
		}

		Super::_erase(e);
		if (Super::_data.size_cache == 0 && Super::_data._root) {
			Super::_data._free_root();
		}
		return true;
	}

	void operator=(const RBSet &p_set) {
		_copy_from(p_set);
	}

	RBSet(const RBSet &p_set) {
		_copy_from(p_set);
	}

	RBSet(std::initializer_list<T> p_init) {
		for (const T &E : p_init) {
			insert(E);
		}
	}

	_FORCE_INLINE_ RBSet() {}
	~RBSet() {}
};

#endif // RB_SET_H
