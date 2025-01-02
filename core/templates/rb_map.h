/**************************************************************************/
/*  rb_map.h                                                              */
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

#ifndef RB_MAP_H
#define RB_MAP_H

#include "core/error/error_macros.h"
#include "core/templates/pair.h"
#include "core/templates/rb_tree.h"


// based on the very nice implementation of rb-trees by:
// https://web.archive.org/web/20120507164830/https://web.mit.edu/~emin/www/source_code/red_black_tree/index.html

template <typename K, typename V, typename A = DefaultAllocator>
class RBMapElement : public RBElement<KeyValue<K, V>, A> {
public:
	K &key() {
		return this->get().key;
	}
	const K &key() const {
		return this->get().key;
	}
	K &value() {
		return this->get().value;
	}
	const K &value() const {
		return this->get().value;
	}
};

template <typename K, typename V, typename C = Comparator<K>, typename A = DefaultAllocator>
class RBMap : public RBTree<RBMapElement<K, V, A>, A> {
public:
	using Element = RBMapElement<K, V, A>;
	typedef KeyValue<K, V> ValueType;
private:
	using Super = RBTree<Element, A>;
	using Color = typename Element::Color;
	using _Data = typename Super::_Data;

	Element *_find(const K &p_key) const {
		Element *node = Super::_data._root->left;
		C less;

		while (node != Super::_data._nil) {
			if (less(p_key, node->_data.key)) {
				node = node->left;
			} else if (less(node->_data.key, p_key)) {
				node = node->right;
			} else {
				return node; // found
			}
		}

		return nullptr;
	}

	Element *_find_closest(const K &p_key) const {
		Element *node = Super::_data._root->left;
		Element *prev = nullptr;
		C less;

		while (node != Super::_data._nil) {
			prev = node;

			if (less(p_key, node->_data.key)) {
				node = node->left;
			} else if (less(node->_data.key, p_key)) {
				node = node->right;
			} else {
				return node; // found
			}
		}

		if (prev == nullptr) {
			return nullptr; // tree empty
		}

		if (less(p_key, prev->_data.key)) {
			prev = prev->_prev;
		}

		return prev;
	}

	Element *_insert(const K &p_key, const V &p_value) {
		Element *new_parent = Super::_data._root;
		Element *node = Super::_data._root->left;
		C less;

		while (node != Super::_data._nil) {
			new_parent = node;

			if (less(p_key, node->_data.key)) {
				node = node->left;
			} else if (less(node->_data.key, p_key)) {
				node = node->right;
			} else {
				node->_data.value = p_value;
				return node; // Return existing node with new value
			}
		}

		typedef KeyValue<K, V> KV;
		Element *new_node = memnew_allocator(Element(KV(p_key, p_value)), A);
		new_node->parent = new_parent;
		new_node->right = Super::_data._nil;
		new_node->left = Super::_data._nil;

		//new_node->data=_data;

		if (new_parent == Super::_data._root || less(p_key, new_parent->_data.key)) {
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

	void _copy_from(const RBMap &p_map) {
		Super::clear();
		// not the fastest way, but safeset to write.
		for (Element *I = p_map.front(); I; I = I->next()) {
			insert(I->key(), I->value.value);
		}
	}

public:
	const Element *find(const K &p_key) const {
		if (!Super::_data._root) {
			return nullptr;
		}

		const Element *res = _find(p_key);
		return res;
	}

	Element *find(const K &p_key) {
		if (!Super::_data._root) {
			return nullptr;
		}

		Element *res = _find(p_key);
		return res;
	}

	const Element *find_closest(const K &p_key) const {
		if (!Super::_data._root) {
			return nullptr;
		}

		const Element *res = _find_closest(p_key);
		return res;
	}

	Element *find_closest(const K &p_key) {
		if (!Super::_data._root) {
			return nullptr;
		}

		Element *res = _find_closest(p_key);
		return res;
	}

	bool has(const K &p_key) const {
		return find(p_key) != nullptr;
	}

	Element *insert(const K &p_key, const V &p_value) {
		if (!Super::_data._root) {
			Super::_data._create_root();
		}
		return _insert(p_key, p_value);
	}

	bool erase(const K &p_key) {
		if (!Super::_data._root) {
			return false;
		}

		Element *e = find(p_key);
		if (!e) {
			return false;
		}

		Super::_erase(e);
		if (Super::_data.size_cache == 0 && Super::_data._root) {
			Super::_data._free_root();
		}
		return true;
	}

	const V &operator[](const K &p_key) const {
		CRASH_COND(!Super::_data._root);
		const Element *e = find(p_key);
		CRASH_COND(!e);
		return e->_data.value;
	}

	V &operator[](const K &p_key) {
		if (!Super::_data._root) {
			Super::_data._create_root();
		}

		Element *e = find(p_key);
		if (!e) {
			e = insert(p_key, V());
		}

		return e->_data.value;
	}

	void operator=(const RBMap &p_map) {
		_copy_from(p_map);
	}

	RBMap(const RBMap &p_map) {
		_copy_from(p_map);
	}

	RBMap(std::initializer_list<KeyValue<K, V>> p_init) {
		for (const KeyValue<K, V> &E : p_init) {
			insert(E.key, E.value);
		}
	}

	_FORCE_INLINE_ RBMap() {}
	~RBMap() {}
};

#endif // RB_MAP_H
