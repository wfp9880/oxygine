#include "Object.h"
#include "Mutex.h"
#include "log.h"
#include "../utils/stringUtils.h"
#include "../winnie_alloc/winnie_alloc.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <typeinfo>
#include "../EventDispatcher.h"

namespace oxygine
{
    extern Mutex mutexAlloc;
    void* fastAlloc(size_t size)
    {
#ifndef USE_MEMORY_POOL
        void* data = malloc(size);
#else
        MutexAutoLock m(mutexAlloc);
        void* data = Winnie::Alloc(size);
#endif
        return data;
    }

    void fastFree(void* data)
    {
#ifndef USE_MEMORY_POOL
        free(data);
#else
        MutexAutoLock m(mutexAlloc);
        Winnie::Free(data);
#endif
    }

    ObjectBase::__createdObjects&   ObjectBase::__getCreatedObjects()
    {
        static __createdObjects __objects;
        return __objects;
    }

    int ObjectBase::_lastID = 0;
    int ObjectBase::_assertCtorID = -1;
    int ObjectBase::_assertDtorID = -1;
    bool _tracingLeaks = false;



    //Mutex mutexDebugList;

    Mutex& getMutex()
    {
        static Mutex mutex;
        return mutex;
    }

#if OBJECT_POOL_ALLOCATOR
    void* PoolObject::operator new(size_t size)
    {
        return fastAlloc(size);
    }

    void PoolObject::operator delete(void* data, size_t size)
    {
#ifdef OX_DEBUG
        memset(data, 0xCDCDCDCD, size);
#endif

        fastFree(data);
    }
#endif

    std::string* ObjectBase::__getName() const
    {
#if DYNAMIC_OBJECT_NAME
        if (!__name)
        {
            __name = (std::string*)fastAlloc(sizeof(std::string));
            new(__name)std::string;
        }
        return __name;
#else
        return &__name;
#endif
    }

    const std::string& ObjectBase::getName() const
    {
        return *__getName();
    }

    bool ObjectBase::isName(const std::string& name) const
    {
        return isName(name.c_str());
    }

    bool ObjectBase::isName(const char* name) const
    {
#if DYNAMIC_OBJECT_NAME
        if (__name && !strcmp(__name->c_str(), name))
            return true;
#else
        if (__name == name)
            return true;
#endif
        return false;
    }

    void ObjectBase::setName(const std::string& name)
    {
        std::string* n = __getName();
        *n = name;
    }

    ObjectBase::ObjectBase(const ObjectBase& src): __id(0), __name(0)
    {
        __addToDebugList(this);

        __userData64 = src.__userData64;
#if DYNAMIC_OBJECT_NAME
        if (src.__name)
            setName(*src.__name);
#else
        setName(src.__name);
#endif
        __generateID();
    }

    void ObjectBase::__removeFromDebugList()
    {
        __removeFromDebugList(this);
    }

    void ObjectBase::__generateID()
    {
        __id = ++_lastID;

        //OX_ASSERT(__id != 266);
#ifdef OX_DEBUG
        OX_ASSERT(_assertCtorID != __id);
#endif
    }

    ObjectBase::ObjectBase(bool assignID): __id(0)
    {
        __userData64 = 0;

#if DYNAMIC_OBJECT_NAME
        __name = 0;
#endif
        __addToDebugList(this);

        if (assignID)
            __generateID();
    }

    void ObjectBase::showAssertInCtor(int id)
    {
        _assertCtorID = id;
    }

    void ObjectBase::showAssertInDtor(int id)
    {
        _assertDtorID = id;
    }

    void ObjectBase::__freeName() const
    {
#if DYNAMIC_OBJECT_NAME
        if (__name)
        {
            using namespace std;

            __name->~string();
            fastFree(__name);
            __name = 0;
        }
#else
        __name.clear();
#endif
    }

    ObjectBase::~ObjectBase()
    {
#ifdef OX_DEBUG
        OX_ASSERT(_assertDtorID != __id);
#endif

#ifdef OXYGINE_DEBUG_TRACE_LEAKS
        __removeFromDebugList(this);
#endif

        __freeName();
    }

    void ObjectBase::__startTracingLeaks()
    {
        _tracingLeaks = true;
    }

    void ObjectBase::__stopTracingLeaks()
    {
        _tracingLeaks = false;
    }

    void ObjectBase::__addToDebugList(ObjectBase* base)
    {
#ifdef OXYGINE_DEBUG_TRACE_LEAKS
        if (_tracingLeaks)
        {
            MutexAutoLock m(getMutex());
            base->__traceLeak = true;
            __getCreatedObjects().push_back(base);
        }
#endif
    }

    void ObjectBase::__removeFromDebugList(ObjectBase* base)
    {
#ifdef OXYGINE_DEBUG_TRACE_LEAKS
        if (_tracingLeaks)
        {
            MutexAutoLock m_(getMutex());
            if (base->__traceLeak)
            {
                base->__traceLeak = false;
                __createdObjects& objs = __getCreatedObjects();
                __createdObjects::iterator i = std::find(objs.begin(), objs.end(), base);
                OX_ASSERT(i != objs.end());
                objs.erase(i);
            }
        }
#endif
    }

    void ObjectBase::dumpObject() const
    {
        const Object* o = dynamic_cast<const Object*>(this);
        std::string name;

        char refs[16] = "-";
        if (o)
        {
            safe_sprintf(refs, "%d", o->_ref_counter);
#if DYNAMIC_OBJECT_NAME
            if (o->__name)
                name = *o->__name;
#else
            name = o->__name;
#endif
        }


        int cbs = 0;
        if (o && o->_ref_counter)
        {
            const EventDispatcher* ed = dynamic_cast<const EventDispatcher*>(o);
            if (ed)
                cbs = ed->getListenersCount();
        }

        logs::messageln("id = %d, name = '%s', typeid = '%s', callbacks = '%d' refs = %s", this->__id, name.c_str(), typeid(*this).name(), cbs, refs);
    }

    void ObjectBase::dumpCreatedObjects()
    {
#ifdef OXYGINE_DEBUG_TRACE_LEAKS
        MutexAutoLock m(getMutex());

        logs::messageln("\n\n\nallocated objects:");
        int n = 0;
        __createdObjects& objs = __getCreatedObjects();
        for (__createdObjects::iterator i = objs.begin(); i != objs.end(); ++i)
        {
            ObjectBase* object = *i;
            //logs::message("%d)", n);
            object->dumpObject();

            ++n;
        }
        logs::message("total: %d -----------------------------\n\n\n", (int)objs.size());
#endif
    }

    Object::Object(const Object& src): ObjectBase(src)
    {
#ifdef OX_DEBUG
        __check = 0xABCDEFAB;
#endif
    }

    Object::Object(bool assignUniqueID): ObjectBase(assignUniqueID)
    {
#ifdef OX_DEBUG
        __check = 0xABCDEFAB;
#endif
    }
#ifdef OX_DEBUG
    void Object::__doCheck()
    {
        OX_ASSERT(this);
        OX_ASSERT(__check == 0xABCDEFAB);
    }
#endif
}