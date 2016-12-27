/** @file *//********************************************************************************************************

                                                 AsynchronousCache.h

                                            Copyright 2006, John J. Bolton
    --------------------------------------------------------------------------------------------------------------

    $Header: //depot/Libraries/AsynchronousCache/AsynchronousCache.h#3 $

    $NoKeywords: $

********************************************************************************************************************/

#pragma once

#include <algorithm>
#include <list>

//! Asynchronous Cache.
//!
//! @param	Element     Type of the elements stored in the cache
//! @param	Key         Type of a key for accessing an element in the cache
//!						KeyType must implement operator==().
//! @param	Handle      Type of an element handle. This is the type of the value returned by Load().
//!						The default type is <tt>void *</tt>.
//!
//! @note	This class is an abstract base class and must be derived from in order to be used.
//! @note	This class (and any derived from it) cannot be copied or assigned.
//!
//! This class provides an asynchronous caching mechanism interface. It has the following characteristics:
//!		- When an element is requested, it will be available sometime in the future.
//!		- Elements must be explicitly released.
//!		- A released element remains in the cache until it is evicted to make room for another element or the cache
//!			is explicitly told to evict it. An evicted element is removed from the cache entirely.
//!		- A request may fail if there is not enough room in the cache.
//!		- An element may be "prefetched". A prefetched element is loaded and immediately released so that it is
//!			in the cache, but it must still be requested.
//!
//! Implementation:
//!
//! The actual cache storage is implemented in the derived class. This class provides a standard interface and
//! manages the storage according to the behaviors described above.
//!
//! When deriving from this class, the following member functions must be overloaded:
//!		- Load()
//!		- Unload()
//!		- HasRoomFor()
//!		- GetElement()
//!
//!	The requirements for these functions are listed in the functions' documentation.

template <typename Element, typename Key, typename Handle = void *>
class AsynchronousCache
{
private:

    // Cache entry
    class Entry
    {
public:

        // Possible states
        enum State
        {
            STATE_REQUESTED,    // Waiting to be loaded
            STATE_PREFETCHED,   // Waiting to be prefetched
            STATE_AVAILABLE,    // Loaded
            STATE_RELEASED      // Waiting to be unloaded
        };

        // Constructor
        Entry(Key const & k, Handle const & t, State s)
            :   key(k),
            state(s),
            handle(t),
            pElement(0)
        {
        }

        // Destructor
        ~Entry()
        {
        }

        Key key;                    // The key for finding this entry
        State state;                // The state of the entry
        Handle handle;              // Handle returned by Load(), used to identify an element.
        Element * pElement;         // The element represented by this entry

        // A functor which returns true if an entry has the specified pointer

        struct pointer_equals
        {
            pointer_equals(Element const * pElement) : m_pElement(pElement) {}
            bool operator ()(Entry const & entry)
            {
                return entry.pElement == m_pElement;
            }

            Element const * m_pElement;
        };

        // A functor which returns true if an entry has the specified key

        struct key_equals
        {
            key_equals(Key const & key) : m_key(key) {}
            bool operator ()(Entry const & entry)
            {
                return entry.key == m_key;
            }

            Key m_key;
        };

        // A functor which returns true if an entry has the specified handle

        struct handle_equals
        {
            handle_equals(Handle const & handle) : m_handle(handle) {}
            bool operator ()(Entry const & entry)
            {
                return entry.handle == m_handle;
            }

            Handle m_handle;
        };
    };

    //! List of cache entries
    typedef std::list<Entry>  EntryList;

public:

    class BackDoor;
    friend class BackDoor;

    typedef ElementType Element;        //!< Type of the element stored in the cache
    typedef KeyType Key;                //!< Type of the element key
    typedef HandleType Handle;          //!< Type of the internal element handle

    //! Default constructor
    AsynchronousCache()     {}

    //! Starts loading a element through the cache
    bool Request(Key const & key);

    //! Notifies the cache that this element may be needed soon
    void Prefetch(Key const & key);

    //! Returns a pointer to an element in the cache (or nullptr if it is not in the cache)
    Element * Get(Key const & key);

    //! Finds an entry in the cache and marks it as no longer used (optionally force eviction)
    void Release(Key const & key, bool forceEviction = false);

    //! Finds an entry in the cache and marks it as no longer used (optionally force eviction)
    void Release(Element const * pElement, bool forceEviction = false);

    //! Returns true when the cache is empty (and no entries are being loaded)
    bool IsEmpty() const;

    //! Removes all elements from the cache
    void Clear();

    //! Returns @c true if the element is in the cache (though possibly released)
    bool IsCached(Key const & key) const;

protected:

    AsynchronousCache(AsynchronousCache const &) = delete;              // Prevent copying
    AsynchronousCache & operator =(AsynchronousCache const &) = delete; // Prevent assignment

    // ****	Functions to override

    //! Starts loading an element with the specified key.
    //!
    //! When an element is requested, the cache will call this function to load it. The handle that is returned is
    //! used as a parameter when unloading the element or checking if it is loaded. Its value is completely
    //! determined by the derived class and the cache makes no attempt to interpret its value. The handle is
    //! intended to provide an efficient and effective way to reference a loaded element directly.
    //!
    //! @param	key		Key identifying the element to load
    //! @return		Returns a handle used to identify the loaded element.
    //! @note	This function must be overridden.

    virtual Handle Load(Key const & key) = 0;

    //! Immediately unloads an element.
    //!
    //! When the cache decides to unload an element (in order to make room for another or when explicitly told),
    //! this function will be called with the element's handle to unload the element. If the element is
    //! currently in the the process of loading, the load must be canceled.
    //!
    //! @param	handle	The handle identifying the entry to unload.
    //! @note	This function must be overridden.

    virtual void Unload(Handle const & handle) = 0;

    //! Returns true if there is room for an entry.
    //!
    //! This function tells if there is room in the cache storage to load the specified entry. The cache manager
    //! relies completely on the value returned by this function -- no other information is used. When an element
    //! is requested, the cache first calls this function to determine if there is room for it. If not, it may
    //! repeatedly unload elements and check if there is room until this function returns <tt>true</tt>.
    //!
    //! @param	key		Key identifying the element to be loaded
    //!
    //! @return	<tt>true</tt>, if the cache storage has room for the specified element
    //! @note	This function must be overridden.

    virtual bool HasRoomFor(Key const & key) = 0;

    //! Returns the address of a loaded element, or nullptr.
    //!
    //! This function returns the address of an element if it is loaded. If it is not loaded (or still loading)
    //! the function returns nullptr.
    //!
    //! @param	handle		Handle of the element to get

    virtual Element * GetElement(Handle const & handle) = 0;

    // ****

private:

    // Finds an entry in the cache and marks it as no longer used (optionally force eviction)
    void Release(typename EntryList::iterator & pEntry, bool forceEviction);

    // Finds an entry by the key, or nullptr if not found
    typename EntryList::iterator Find(Key const & key);

    // Finds an entry by the handle, or nullptr if not found
    typename EntryList::iterator Find(Handle const & handle);

    // Finds an entry by the address of the element, or nullptr if not found
    typename EntryList::iterator Find(Element const * pElement);

    // Evicts enough entries in the cache to make room for a new one. Returns true if successful.
    bool MakeRoomForNewEntry(Key const & key);

    // Removes an entry from the cache. Returns the iterator of the next entry.
    typename EntryList::iterator Evict(typename EntryList::iterator & pEntry);

    // Loads an element into the cache (asynchronously). Returns the entry's iterator.
    typename EntryList::iterator Fetch(Key const & key, Entry::State state);

    // Reloads an evicted element
    void Reload(typename EntryList::iterator & pEntry);

    EntryList m_entries;            // The cache entries
};

template <typename Element, typename Key, typename Handle = void *>
class AsynchronousCache<Element, Key, Handle>::BackDoor
{
public:

    typedef AsynchronousCache<Element, Key, Handle>   Target;
    typedef typename Target::Entry Entry;
    typedef typename Target::EntryList EntryList;

    BackDoor(Target * target)
        : m_target(target)
    {
    }

    typename EntryList::iterator Find(Key const & key) const { return m_target->Find(key); }
    typename EntryList::iterator Find(Handle const & handle) const { return m_target->Find(key); }
    typename EntryList::iterator Find(Element const * pElement) const { return m_target->Find(key); }
    typename EntryList & GetEntries() const { return m_target->m_entries; }
    Element * GetElement(Handle const & handle) const { return m_target->GetElement(handle); }

private:

    Target * m_target;
};

//! This function starts loading a element into the cache. When it is available, Get() will returns a pointer to
//! it, but until then, Get() will return 0. If a requested element is released before it is loaded, the request
//! will be canceled.
//!
//! @param	key		Element to load
//!
//! @return		@c false, if there is no room in the cache to load the element
//!
//! @note		Requesting an available or requested element does nothing.

template <typename Element, typename Key, typename Handle>
bool AsynchronousCache<Element, Key, Handle>::Request(Key const & key)
{
    bool ok;

    // Check if the element is already in the cache. If it is, then reload it if it is being evicted. If it is not
    // already in the cache, then load the element.

    EntryList::iterator pEntry = Find(key);

    if (pEntry != m_entries.end())
    {
        // Check the state of the entry and do the appropriate thing.

        switch (pEntry->state)
        {
            case Entry::STATE_REQUESTED:    // Not available yet, nothing else to do
            case Entry::STATE_AVAILABLE:    // Already available, nothing to do
                break;

            case Entry::STATE_PREFETCHED:
            {
                Element * pElement = GetElement(pEntry->handle);
                if (pElement != 0)
                {
                    pEntry->pElement = pElement;
                    pEntry->state    = Entry::STATE_AVAILABLE;
                }
                else
                {
                    pEntry->state = Entry::STATE_REQUESTED;
                }
                break;
            }
            case Entry::STATE_RELEASED:
                Reload(pEntry);
                break;
        }

        ok = true;
    }
    else
    {
        ok = (Fetch(key, Entry::STATE_REQUESTED) != m_entries.end());
    }

    return ok;
}

//! This function starts loading a element into the cache, however it is not available until it is also requested.
//! If a prefetched element is released before it is loaded, the load is canceled. The element may not be loaded
//! if there is no room in the cache.
//!
//! @param	key		Element to prefetch
//!
//! @note	Prefetching an available, requested, or prefetched element does nothing.

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Prefetch(Key const & key)
{
    // Check if the element is already in the cache. If it is released, then make it the last to be evicted.
    // If it is not already in the cache, then load it it and release it.

    EntryList::iterator pEntry = Find(key);

    if (pEntry != m_entries.end())
    {
        // Check the state of the entry and do the appropriate thing.

        switch (pEntry->state)
        {
            case Entry::STATE_REQUESTED: // Already requested, nothing else to do
            case Entry::STATE_AVAILABLE: // Already available, nothing to do
                break;

            case Entry::STATE_PREFETCHED:
            case Entry::STATE_RELEASED:

                // A prefetched entry is still marked as released, so there is no change in its state. Just move the
                // entry to the end of the list so it is the last to be evicted.

                m_entries.splice(m_entries.end(), m_entries, pEntry);
                break;
        }
    }
    else
    {
        Fetch(key, Entry::STATE_PREFETCHED);
    }
}

//! This function returns a pointer to an element in the cache. After an element is requested, Get() will return
//! 0 until the element is available. An element that has never been requested will always return 0.
//!
//! @param	key		Element to access
//!
//! @return		Pointer to the element, or 0 if the element is not in the cache.

template <typename Element, typename Key, typename Handle>
Element * AsynchronousCache<Element, Key, Handle>::Get(Key const & key)
{
    Element * result;

    EntryList::iterator pEntry = Find(key);

    // If the element is in the list, then check if it is available or not. Otherwise, return 0.

    if (pEntry != m_entries.end())
    {
        // If it was requested, see if it is available. If it is, then update the state

        if (pEntry->state == Entry::STATE_REQUESTED)
        {
            Element * pElement = GetElement(pEntry->handle);
            if (pElement != 0)
            {
                pEntry->pElement = pElement;
                pEntry->state    = Entry::STATE_AVAILABLE;
            }
        }

        result = pEntry->pElement;
    }
    else
    {
        result = 0;
    }

    return result;
}

//! This function "releases" a cache element. Once the element is released, it is no longer usable and may be
//! evicted from the cache at any time. Elements must be released in order to be evicted from the cache. If the
//! cache has a limited size, then elements must be released in order to make room for new elements.
//!
//! @param	key				Element to release
//! @param	forceEviction	If @c true, the element is immediately removed from the cache storage.
//!
//! @note	Releasing a released element by key does nothing.

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Release(Key const & key, bool forceEviction /* = false*/)
{
    EntryList::iterator pEntry = Find(key);

    if (pEntry != m_entries.end())
    {
        Release(pEntry, forceEviction);
    }
}

//! This function "releases" a cache element. Once the element is released, it is no longer usable and may be
//! evicted from the cache at any time. Elements must be released in order to be evicted from the cache. If the
//! cache has a limited size, then elements must be released in order to make room for new elements.
//!
//! @param	pElement		Element to release
//! @param	forceEviction	If @c true, the element is immediately removed from the cache storage.
//!
//! @warn	Addresses are not unique, so specifying the address of a previously released element may release a
//!			different element.

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Release(Element const * pElement, bool forceEviction /* = false*/)
{
    EntryList::iterator pEntry = Find(pElement);

    if (pEntry != m_entries.end())
    {
        Release(pEntry, forceEviction);
    }
}

//! This function returns @c true if there are no elements in the cache (whether active or released).

template <typename Element, typename Key, typename Handle>
bool AsynchronousCache<Element, Key, Handle>::IsEmpty() const
{
    bool empty = m_entries.empty();
}

//! This function evicts all entries from the cache. An "evicted" element is removed from the cache entirely.

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Clear()
{
    // Go through the list and evict every entry

    EntryList::iterator pEntry = m_entries.begin();
    while (pEntry != m_entries.end())
    {
        pEntry = Evict(pEntry);
    }
}

//! This function returns @c true if the specified element is in the cache, even if it is released. Elements that
//! are loading or prefetching will return @c false.
//!
//! @param	key		Element to check

template <typename Element, typename Key, typename Handle>
bool AsynchronousCache<Element, Key, Handle>::IsCached(Key const & key) const
{
    EntryList::iterator pEntry = const_cast<AsynchronousCache<Element, Key, Handle> *>(this)->Find(key);
    bool isCached = (pEntry != m_entries.end() &&
                     (pEntry->state == Entry::STATE_AVAILABLE || pEntry->state == Entry::STATE_RELEASED));

    return isCached;
}

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Release(typename EntryList::iterator & pEntry, bool forceEviction)
{
    // If this entry is not yet available or it is a forced eviction, then go ahead and evict now.
    // Otherwise, mark it as being evicted and move it to the end (so it is unloaded after any
    // entry that was evicted before it).

    if (pEntry->state == Entry::STATE_REQUESTED || forceEviction)
    {
        Evict(pEntry);
    }
    else if (pEntry->state == Entry::STATE_AVAILABLE)
    {
        pEntry->state = Entry::STATE_RELEASED;
        m_entries.splice(m_entries.end(), m_entries, pEntry);
    }
    else
    {
        // error can only release requested or available elements
    }
}

template <typename Element, typename Key, typename Handle>
typename std::list<typename AsynchronousCache<Element, Key, Handle>::Entry>::iterator AsynchronousCache<Element, Key, Handle>::Find(
    Key const & key)
{
    // Return an element with a matching key, or m_entries.end()

    return std::find_if(m_entries.begin(), m_entries.end(), Entry::key_equals(key));
}

template <typename Element, typename Key, typename Handle>
typename std::list<typename AsynchronousCache<Element, Key, Handle>::Entry>::iterator AsynchronousCache<Element, Key, Handle>::Find(
    Handle const & handle)
{
    // Return an element with a matching handle, or m_entries.end()

    return std::find_if(m_entries.begin(), m_entries.end(), Entry::handle_equals(handle));
}

template <typename Element, typename Key, typename Handle>
typename std::list<typename AsynchronousCache<Element, Key, Handle>::Entry>::iterator AsynchronousCache<Element, Key, Handle>::Find(
    Element const * pElement)
{
    // Return an element with a matching address, or m_entries.end()

    return std::find_if(m_entries.begin(), m_entries.end(), Entry::pointer_equals(pElement));
}

template <typename Element, typename Key, typename Handle>
bool AsynchronousCache<Element, Key, Handle>::MakeRoomForNewEntry(Key const & key)
{
    // Go through the list from front to back evicting entries until there is room for the entry
    // or there are no more entries to evict.

    EntryList::iterator pEntry;

    // First, evict released elements.

    pEntry = m_entries.begin();

    while (!HasRoomFor(key) && pEntry != m_entries.end())
    {
        if (pEntry->state == Entry::STATE_RELEASED || pEntry->state == Entry::STATE_PREFETCHED)
        {
            pEntry = Evict(pEntry);
        }
        else
        {
            ++pEntry;
        }
    }

    return HasRoomFor(key);
}

template <typename Element, typename Key, typename Handle>
typename std::list<typename AsynchronousCache<Element, Key, Handle>::Entry>::iterator AsynchronousCache<Element, Key,
                                                                                                        Handle>::Evict(
    typename EntryList::iterator & pEntry)
{
    Unload(pEntry->handle);                     // Unload the data
    return m_entries.erase(pEntry);             // Erase the cache entry
}

template <typename Element, typename Key, typename Handle>
typename std::list<typename AsynchronousCache<Element, Key, Handle>::Entry>::iterator AsynchronousCache<Element, Key,
                                                                                                        Handle>::Fetch(
    Key const &  key,
    Entry::State state)
{
    // If the cache has reached its limit, then evict elements to make room for the one to be loaded.

    EntryList::iterator pEntry = m_entries.end();

    if (MakeRoomForNewEntry(key))
    {
        // Start loading the stream

        Handle handle = Load(key);

        // Add the entry to the list. Add it to the back so it is the last to be evicted.

        pEntry = m_entries.insert(m_entries.end(), Entry(key, handle, state));
    }

    return pEntry;
}

template <typename Element, typename Key, typename Handle>
void AsynchronousCache<Element, Key, Handle>::Reload(typename EntryList::iterator & pEntry)
{
    pEntry->state = Entry::STATE_AVAILABLE;
}
