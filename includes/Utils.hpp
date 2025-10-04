#ifndef UTILS_HPP
# define UTILS_HPP

# include <map>

// Simple RAII wrapper for managing maps of pointers
template <typename Key, typename Value>
class PointerMap {
private:
    std::map<Key, Value*> map_;

public:
    typedef typename std::map<Key, Value*>::iterator iterator;
    typedef typename std::map<Key, Value*>::const_iterator const_iterator;

    PointerMap() {}
    
    ~PointerMap() {
        clear();
    }

    // Add a pointer (takes ownership)
    void insert(const Key& key, Value* ptr) {
        if (map_.count(key)) {
            delete map_[key];
        }
        map_[key] = ptr;
    }

    // Remove and delete a pointer
    void erase(const Key& key) {
        if (map_.count(key)) {
            delete map_[key];
            map_.erase(key);
        }
    }

    // Clear all pointers
    void clear() {
        for (iterator it = map_.begin(); it != map_.end(); ++it) {
            delete it->second;
        }
        map_.clear();
    }

    // Access (non-owning)
    Value* operator[](const Key& key) {
        return map_[key];
    }

    const Value* operator[](const Key& key) const {
        typename std::map<Key, Value*>::const_iterator it = map_.find(key);
        return (it != map_.end()) ? it->second : NULL;
    }

    // Check existence
    bool count(const Key& key) const {
        return map_.count(key);
    }

    // Iterators
    iterator begin() { return map_.begin(); }
    iterator end() { return map_.end(); }
    const_iterator begin() const { return map_.begin(); }
    const_iterator end() const { return map_.end(); }

    // Size
    size_t size() const { return map_.size(); }

    // Get underlying map (for compatibility with existing code)
    std::map<Key, Value*>& getMap() { return map_; }
    const std::map<Key, Value*>& getMap() const { return map_; }

private:
    // Prevent copying to avoid double-deletion
    PointerMap(const PointerMap&);
    PointerMap& operator=(const PointerMap&);
};

#endif