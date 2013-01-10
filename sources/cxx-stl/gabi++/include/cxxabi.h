// Copyright (C) 2011 The Android Open Source Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#ifndef __GABIXX_CXXABI_H__
#define __GABIXX_CXXABI_H__

#include <exception>
#include <typeinfo>
#include <unwind.h>

namespace __cxxabiv1
{
  // Derived types of type_info below are based on 2.9.5 of C++ ABI.

  // Typeinfo for fundamental types.
  class __fundamental_type_info : public std::type_info
  {
  public:
    virtual ~__fundamental_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for array types.
  class __array_type_info : public std::type_info
  {
  public:
    virtual ~__array_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for function types.
  class __function_type_info : public std::type_info
  {
  public:
    virtual ~__function_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                           void*& adjustedPtr) const;
  };

  // Typeinfo for enum types.
  class __enum_type_info : public std::type_info
  {
  public:
    virtual ~__enum_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                           void*& adjustedPtr) const;
  };


  class __class_type_info;

  // Used in __vmi_class_type_info
  struct __base_class_type_info
  {
  public:
    const __class_type_info *__base_type;

    long __offset_flags;

    enum __offset_flags_masks {
      __virtual_mask = 0x1,
      __public_mask = 0x2,
      __offset_shift = 8   // lower 8 bits are flags
    };

    bool is_virtual() const {
      return (__offset_flags & __virtual_mask) != 0;
    }

    bool is_public() const {
      return (__offset_flags & __public_mask) != 0;
    }

    // FIXME: Right-shift of signed integer is implementation dependent.
    // GCC Implement is as signed (as we expect)
    long offset() const {
      return __offset_flags >> __offset_shift;
    }

    long flags() const {
      return __offset_flags & ((1 << __offset_shift) - 1);
    }
  };

  // Helper struct to support catch-clause match
  struct __UpcastInfo {
    enum ContainedStatus {
      unknown = 0,
      has_public_contained,
      has_ambig_or_not_public
    };

    ContainedStatus status;
    const __class_type_info* base_type;
    void* adjustedPtr;
    unsigned int premier_flags;
    bool nullobj_may_conflict;

    __UpcastInfo(const __class_type_info* type);
  };

  // Typeinfo for classes with no bases.
  class __class_type_info : public std::type_info
  {
  public:
    virtual ~__class_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                          void*& adjustedPtr) const;

    enum class_type_info_code {
      CLASS_TYPE_INFO_CODE,
      SI_CLASS_TYPE_INFO_CODE,
      VMI_CLASS_TYPE_INFO_CODE
    };

    virtual class_type_info_code
      code() const { return CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;

  protected:
    bool self_class_type_match(const __class_type_info* base_type,
                               void*& adjustedPtr,
                               __UpcastInfo& info) const;
  };

  // Typeinfo for classes containing only a single, public, non-virtual base at
  // offset zero.
  class __si_class_type_info : public __class_type_info
  {
  public:
    virtual ~__si_class_type_info();
    const __class_type_info *__base_type;

    virtual __class_type_info::class_type_info_code
      code() const { return SI_CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;
  };


  // Typeinfo for classes with bases that do not satisfy the
  // __si_class_type_info constraints.
  class __vmi_class_type_info : public __class_type_info
  {
  public:
    virtual ~__vmi_class_type_info();
    unsigned int __flags;
    unsigned int __base_count;
    __base_class_type_info __base_info[1];

    enum __flags_masks {
      __non_diamond_repeat_mask = 0x1,
      __diamond_shaped_mask = 0x2,
    };

    virtual __class_type_info::class_type_info_code
      code() const { return VMI_CLASS_TYPE_INFO_CODE; }

    virtual bool walk_to(const __class_type_info* base_type,
                         void*& adjustedPtr,
                         __UpcastInfo& info) const;
  };

  class __pbase_type_info : public std::type_info
  {
  public:
    virtual ~__pbase_type_info();
    virtual bool can_catch(const std::type_info* thrown_type,
                          void*& adjustedPtr) const;
    unsigned int __flags;
    const std::type_info *__pointee;

    enum __masks {
      __const_mask = 0x1,
      __volatile_mask = 0x2,
      __restrict_mask = 0x4,
      __incomplete_mask = 0x8,
      __incomplete_class_mask = 0x10
    };


    virtual bool can_catch_typeinfo_wrapper(const std::type_info* thrown_type,
                                            void*& adjustedPtr,
                                            unsigned tracker) const;

  protected:
    enum __constness_tracker_status {
      first_time_init = 0x1,
      keep_constness = 0x2,
      after_gap = 0x4         // after one non-const qualified,
                              // we cannot face const again in future
    };

  private:
    bool can_catch_ptr(const __pbase_type_info *thrown_type,
                       void *&adjustedPtr,
                       unsigned tracker) const;

    // Return true if making decision done.
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const = 0;
  };

  class __pointer_type_info : public __pbase_type_info
  {
  public:
    virtual ~__pointer_type_info();

  private:
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const;
  };

  class __pointer_to_member_type_info : public __pbase_type_info
  {
  public:
    __class_type_info* __context;

    virtual ~__pointer_to_member_type_info();

  private:
    virtual bool do_can_catch_ptr(const __pbase_type_info* thrown_type,
                                  void*& adjustedPtr,
                                  unsigned tracker,
                                  bool& result) const;
  };


  extern "C" {

    // Compatible with GNU C++
    const uint64_t __gxx_exception_class = 0x474E5543432B2B00; // GNUCC++\0

    // TODO: Support dependent exception
    // TODO: Support C++0x exception propagation
    // http://sourcery.mentor.com/archives/cxx-abi-dev/msg01924.html
    struct __cxa_exception {
      std::type_info* exceptionType;
      void (*exceptionDestructor)(void*);
      std::unexpected_handler unexpectedHandler;
      std::terminate_handler terminateHandler;
      __cxa_exception* nextException;

      int handlerCount;
#ifdef __arm__
      /**
       * ARM EHABI requires the unwind library to keep track of exceptions
       * during cleanups.  These support nesting, so we need to keep a list of
       * them.
       */
      __cxa_exception* nextCleanup;
      int cleanupCount;
#endif
      int handlerSwitchValue;
      const uint8_t* actionRecord;
      const uint8_t* languageSpecificData;
      void* catchTemp;
      void* adjustedPtr;

      _Unwind_Exception unwindHeader; // must be last
    };

    struct __cxa_eh_globals {
      __cxa_exception* caughtExceptions;
      unsigned int uncaughtExceptions;
#ifdef __arm__
      __cxa_exception* cleanupExceptions;
#endif
    };

    struct __cxa_thread_info {
      std::unexpected_handler unexpectedHandler;
      std::terminate_handler terminateHandler;
      _Unwind_Exception* currentCleanup;

      __cxa_eh_globals globals;
    };

    __cxa_eh_globals* __cxa_get_globals();
    __cxa_eh_globals* __cxa_get_globals_fast();

    void* __cxa_allocate_exception(size_t thrown_size);
    void __cxa_free_exception(void* thrown_exception);

    void __cxa_throw(void* thrown_exception, std::type_info* tinfo, void (*dest)(void*));
    void __cxa_rethrow();

    void* __cxa_begin_catch(void* exceptionObject);
    void __cxa_end_catch();

    bool __cxa_begin_cleanup(_Unwind_Exception*);
    void __cxa_end_cleanup();

    void __cxa_bad_cast();
    void __cxa_bad_typeid();

    void* __cxa_get_exception_ptr(void* exceptionObject);

    void __cxa_pure_virtual();

#ifdef __arm__
    typedef enum {
      ctm_failed = 0,
      ctm_succeeded = 1,
      ctm_succeeded_with_ptr_to_base = 2
    } __cxa_type_match_result;

    __cxa_type_match_result __cxa_type_match(_Unwind_Control_Block* ucbp,
                                             const std::type_info* rttip,
                                             bool is_reference_type,
                                             void** matched_object);
#endif

  } // extern "C"

} // namespace __cxxabiv1

namespace abi = __cxxabiv1;

#endif /* defined(__GABIXX_CXXABI_H__) */

