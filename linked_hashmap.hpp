/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
	struct Node {
		value_type *data;
		Node *prev;
		Node *next;
		Node *hashNext;

		Node(const value_type &val, Node *p = nullptr, Node *n = nullptr, Node *h = nullptr)
			: data(new value_type(val)), prev(p), next(n), hashNext(h) {}

		~Node() {
			delete data;
		}
	};

	Node **buckets;
	size_t bucketCount;
	size_t elementCount;
	Node *head;
	Node *tail;

	static constexpr double LOAD_FACTOR = 0.75;

	Hash hashFunc;
	Equal equalFunc;

	void initBuckets(size_t count) {
		bucketCount = count;
		buckets = new Node*[bucketCount];
		for (size_t i = 0; i < bucketCount; ++i) {
			buckets[i] = nullptr;
		}
	}

	void clearBuckets() {
		for (size_t i = 0; i < bucketCount; ++i) {
			Node *current = buckets[i];
			while (current) {
				Node *next = current->hashNext;
				delete current;
				current = next;
			}
			buckets[i] = nullptr;
		}
	}

	void rehash() {
		Node **oldBuckets = buckets;

		size_t newBucketCount = bucketCount * 2;
		initBuckets(newBucketCount);

		// Reinsert all nodes from the linked list
		Node *current = head;
		while (current) {
			// Reinsert into new hash table
			size_t index = hashFunc(current->data->first) % bucketCount;
			current->hashNext = buckets[index];
			buckets[index] = current;
			current = current->next;
		}

		delete[] oldBuckets;
	}

public:
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		linked_hashmap *map;
		Node *node;

		iterator(linked_hashmap *m, Node *n) : map(m), node(n) {}

		friend class linked_hashmap;
		friend class const_iterator;

	public:
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the
		// iterator points to.
		// STL algorithms and containers may use these type_traits (e.g. the following
		// typedef) to work properly.
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;


		iterator() : map(nullptr), node(nullptr) {}
		iterator(const iterator &other) : map(other.map), node(other.node) {}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			iterator tmp = *this;
			node = node->next;
			return tmp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			node = node->next;
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if (node == nullptr || node == map->head) {
				throw invalid_iterator();
			}
			iterator tmp = *this;
			node = node->prev;
			return tmp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if (node == nullptr || node == map->head) {
				throw invalid_iterator();
			}
			node = node->prev;
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			return *(node->data);
		}
		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator==(const const_iterator &rhs) const;
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}
		bool operator!=(const const_iterator &rhs) const;

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			if (node == nullptr) {
				return nullptr;
			}
			return node->data;
		}
	};

	class const_iterator {
		const linked_hashmap *map;
		Node *node;

		const_iterator(const linked_hashmap *m, Node *n) : map(m), node(n) {}

		friend class linked_hashmap;
		friend class iterator;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::output_iterator_tag;

		const_iterator() : map(nullptr), node(nullptr) {}
		const_iterator(const const_iterator &other) : map(other.map), node(other.node) {}
		const_iterator(const iterator &other) : map(other.map), node(other.node) {}

		const_iterator operator++(int) {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			const_iterator tmp = *this;
			node = node->next;
			return tmp;
		}
		const_iterator & operator++() {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			node = node->next;
			return *this;
		}
		const_iterator operator--(int) {
			if (node == nullptr || node == map->head) {
				throw invalid_iterator();
			}
			const_iterator tmp = *this;
			node = node->prev;
			return tmp;
		}
		const_iterator & operator--() {
			if (node == nullptr || node == map->head) {
				throw invalid_iterator();
			}
			node = node->prev;
			return *this;
		}
		const value_type & operator*() const {
			if (node == nullptr) {
				throw invalid_iterator();
			}
			return *(node->data);
		}
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}
		bool operator!=(const const_iterator &rhs) const {
			return node != rhs.node;
		}
		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}
		const value_type* operator->() const noexcept {
			if (node == nullptr) {
				return nullptr;
			}
			return node->data;
		}
	};

	/**
	 * TODO two constructors
	 */
	linked_hashmap() : bucketCount(16), elementCount(0), head(nullptr), tail(nullptr), hashFunc(Hash()), equalFunc(Equal()) {
		initBuckets(bucketCount);
	}

	linked_hashmap(const linked_hashmap &other) : bucketCount(other.bucketCount), elementCount(0), head(nullptr), tail(nullptr), hashFunc(other.hashFunc), equalFunc(other.equalFunc) {
		initBuckets(bucketCount);

		// Copy all elements
		Node *curr = other.head;
		while (curr) {
			insert(*(curr->data));
			curr = curr->next;
		}
	}

	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) {
			return *this;
		}
		clear();
		if (buckets) {
			delete[] buckets;
			buckets = nullptr;
		}

		bucketCount = other.bucketCount;
		elementCount = 0;
		hashFunc = other.hashFunc;
		equalFunc = other.equalFunc;
		head = tail = nullptr;
		initBuckets(bucketCount);

		// Copy all elements
		Node *curr = other.head;
		while (curr) {
			insert(*(curr->data));
			curr = curr->next;
		}

		return *this;
	}

	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
		clear();
		if (buckets) {
			delete[] buckets;
		}
	}

	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		iterator it = find(key);
		if (it == end()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	const T & at(const Key &key) const {
		const_iterator it = find(key);
		if (it == cend()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		iterator it = find(key);
		if (it == end()) {
			// This will fail for types without default constructor
			// But we need to create a default value for the insert
			T temp{};  // Try default construction
			auto result = insert(value_type(key, temp));
			return result.first->second;
		}
		return it->second;
	}

	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		return at(key);
	}

	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		return iterator(this, head);
	}

	const_iterator cbegin() const {
		return const_iterator(this, head);
	}

	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(this, nullptr);
	}

	const_iterator cend() const {
		return const_iterator(this, nullptr);
	}

	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return elementCount == 0;
	}

	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return elementCount;
	}

	/**
	 * clears the contents
	 */
	void clear() {
		if (head) {
			Node *current = head;
			while (current) {
				Node *next = current->next;
				delete current;
				current = next;
			}
			head = tail = nullptr;
		}
		if (buckets) {
			clearBuckets();
		}
		elementCount = 0;
	}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		// Check if key already exists
		iterator existing = find(value.first);
		if (existing != end()) {
			return pair<iterator, bool>(existing, false);
		}

		// Check if rehash is needed
		if (elementCount >= bucketCount * LOAD_FACTOR) {
			rehash();
		}

		// Create new node
		Node *newNode = new Node(value);

		// Insert into hash table
		size_t index = hashFunc(value.first) % bucketCount;
		newNode->hashNext = buckets[index];
		buckets[index] = newNode;

		// Insert into linked list
		if (!head) {
			head = tail = newNode;
		} else {
			tail->next = newNode;
			newNode->prev = tail;
			tail = newNode;
		}

		elementCount++;
		return pair<iterator, bool>(iterator(this, newNode), true);
	}

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos == end() || pos.node == nullptr) {
			throw invalid_iterator();
		}

		Node *target = pos.node;

		// Remove from hash table
		size_t index = hashFunc(target->data->first) % bucketCount;
		Node **current = &buckets[index];
		while (*current != nullptr) {
			if (*current == target) {
				*current = target->hashNext;
				break;
			}
			current = &((*current)->hashNext);
		}

		// Remove from linked list
		if (target->prev) {
			target->prev->next = target->next;
		} else {
			head = target->next;
		}

		if (target->next) {
			target->next->prev = target->prev;
		} else {
			tail = target->prev;
		}

		delete target;
		elementCount--;
	}

	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
		return find(key) != cend() ? 1 : 0;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		size_t index = hashFunc(key) % bucketCount;
		Node *current = buckets[index];
		while (current != nullptr) {
			if (equalFunc(current->data->first, key)) {
				return iterator(this, current);
			}
			current = current->hashNext;
		}
		return end();
	}

	const_iterator find(const Key &key) const {
		size_t index = hashFunc(key) % bucketCount;
		Node *current = buckets[index];
		while (current != nullptr) {
			if (equalFunc(current->data->first, key)) {
				return const_iterator(this, current);
			}
			current = current->hashNext;
		}
		return cend();
	}
};

template<class Key, class T, class Hash, class Equal>
bool linked_hashmap<Key, T, Hash, Equal>::iterator::operator==(const const_iterator &rhs) const {
	return node == rhs.node;
}

template<class Key, class T, class Hash, class Equal>
bool linked_hashmap<Key, T, Hash, Equal>::iterator::operator!=(const const_iterator &rhs) const {
	return node != rhs.node;
}

}

#endif