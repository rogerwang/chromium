// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_CPP_OUTPUT_TRAITS_H_
#define PPAPI_CPP_OUTPUT_TRAITS_H_

#include "ppapi/c/pp_resource.h"
#include "ppapi/cpp/array_output.h"

/// @file
/// This file defines internal templates for defining how data is passed to the
/// browser via output parameters and how to convert that data to the
/// corresponding C++ object types.
///
/// It is used by the callback system, it should not be necessary for end-users
/// to use these templates directly.

struct PP_Var;

namespace pp {

class Resource;
class Var;

namespace internal {

// This goop is a trick used to implement a template that can be used to
// determine if a given class is the base class of another given class. It is
// used in the resource object partial specialization below.
template<typename, typename> struct IsSame {
  static bool const value = false;
};
template<typename A> struct IsSame<A, A> {
  static bool const value = true;
};
template<typename Base, typename Derived> struct IsBaseOf {
  static Derived* CreateDerived();
  static char (&Check(Base*))[1];
  static char (&Check(...))[2];
  static bool const value = sizeof Check(CreateDerived()) == 1 &&
                            !IsSame<Base const, void const>::value;
};

// Template to optionally derive from a given base class T if the given
// predicate P is true.
template <class T, bool P> struct InheritIf {};
template <class T> struct InheritIf<T, true> : public T {};

// Single output parameters ----------------------------------------------------

// Output traits for all "plain old data" (POD) types. It is implemented to
// pass a pointer to the browser as an output parameter.
//
// This is used as a base class for the general CallbackOutputTraits below in
// the case where T is not a resource.
template<typename T>
struct GenericCallbackOutputTraits {
  // The type passed to the PPAPI C API for this parameter. For normal out
  // params, we pass a pointer to the object so the browser can write into it.
  typedef T* APIArgType;

  // The type used to store the value. This is used internally in asynchronous
  // callbacks by the CompletionCallbackFactory to have the browser write into
  // a temporary value associated with the callback, which is passed to the
  // plugin code when the callback is issued.
  typedef T StorageType;

  // Converts a "storage type" to a value that can be passed to the browser as
  // an output parameter. This just takes the address to convert the value to
  // a pointer.
  static inline APIArgType StorageToAPIArg(StorageType& t) { return &t; }

  // Converts the "storage type" to the value we pass to the plugin for
  // callbacks. This doesn't actually need to do anything in this case,
  // it's needed for some of more complex template specializations below.
  static inline T& StorageToPluginArg(StorageType& t) { return t; }
};

// Output traits for all resource types. It is implemented to pass a
// PP_Resource* as an output parameter to the browser, and convert to the
// given resource object type T when passing to the plugin.
//
// Note that this class is parameterized by the resource object, for example
// ResourceCallbackOutputTraits<pp::FileRef>. This is used as a base class for
// CallbackOutputTraits below for the case where T is a derived class of
// pp::Resource.
template<typename T>
struct ResourceCallbackOutputTraits {
  // To call the browser, we just pass a PP_Resource pointer as the out param.
  typedef PP_Resource* APIArgType;
  typedef PP_Resource StorageType;

  static inline APIArgType StorageToAPIArg(StorageType& t) {
    return &t;
  }

  // Converts the PP_Resource to a pp::* object, passing any reference counted
  // object along with it. This must only be called once since there will only
  // be one reference that the browser has assigned to us for the out param!
  // When calling into the plugin, convert the PP_Resource into the requested
  // resource object type.
  static inline T StorageToPluginArg(StorageType& t) {
    return T(PASS_REF, t);
  }
};

// The general templatized base class for all CallbackOutputTraits. This class
// covers both resources and POD (ints, structs, etc.) by inheriting from the
// appropriate base class depending on whether the given type derives from
// pp::Resource. This trick allows us to do this once rather than writing
// specializations for every resource object type.
template<typename T>
struct CallbackOutputTraits
    : public InheritIf<GenericCallbackOutputTraits<T>,
                       !IsBaseOf<Resource, T>::value>,
      public InheritIf<ResourceCallbackOutputTraits<T>,
                       IsBaseOf<Resource, T>::value> {
};

// A specialization of CallbackOutputTraits for pp::Var output parameters.
// It passes a PP_Var* to the browser and converts to a pp::Var when passing
// to the plugin.
template<>
struct CallbackOutputTraits<Var> {
  // To call the browser, we just pass a PP_Var* as an output param.
  typedef PP_Var* APIArgType;
  typedef PP_Var StorageType;

  static inline APIArgType StorageToAPIArg(StorageType& t) {
    return &t;
  }

  // Converts the PP_Var to a pp::Var object, passing any reference counted
  // object along with it. This must only be called once since there will only
  // be one reference that the browser has assigned to us for the out param!
  static inline pp::Var StorageToPluginArg(StorageType& t) {
    return Var(PASS_REF, t);
  }
};

// Array output parameters -----------------------------------------------------

// Output traits for vectors of all "plain old data" (POD) types. It is
// implemented to pass a pointer to the browser as an output parameter.
//
// This is used as a base class for the general vector CallbackOutputTraits
// below in the case where T is not a resource.
template<typename T>
struct GenericVectorCallbackOutputTraits {
  // All arrays are output via a PP_ArrayOutput type.
  typedef PP_ArrayOutput* APIArgType;

  // We store the array as this adapter which combines the PP_ArrayOutput
  // structure with the underlying std::vector that it will write into.
  typedef ArrayOutputAdapterWithStorage<T> StorageType;

  // Retrieves the PP_ArrayOutput interface for our vector object that the
  // browser will use to write into our code.
  static inline APIArgType StorageToAPIArg(StorageType& t) {
    return t.pp_array_output();
  }

  // Retrieves the underlying vector that can be passed to the plugin.
  static inline std::vector<T>& StorageToPluginArg(StorageType& t) {
    return t.output();
  }
};

// Output traits for all vectors of resource types. It is implemented to pass
// a PP_Resource* as an output parameter to the browser, and convert to the
// given resource object type T when passing to the plugin.
//
// Note that this class is parameterized by the resource object, for example
// ResourceVectorCallbackOutputTraits<pp::FileRef>. This is used as a base
// class for CallbackOutputTraits below for the case where T is a derived
// class of pp::Resource.
template<typename T>
struct ResourceVectorCallbackOutputTraits {
  typedef PP_ArrayOutput* APIArgType;
  typedef ResourceArrayOutputAdapterWithStorage<T> StorageType;

  static inline APIArgType StorageToAPIArg(StorageType& t) {
    return t.pp_array_output();
  }
  static inline std::vector<T>& StorageToPluginArg(StorageType& t) {
    return t.output();
  }
};

// Specialization of CallbackOutputTraits for vectors. This struct covers both
// arrays of resources and arrays of POD (ints, structs, etc.) by inheriting
// from the appropriate base class depending on whether the given type derives
// from pp::Resource. This trick allows us to do this once rather than writing
// specilalizations for every resource object type.
template<typename T>
struct CallbackOutputTraits< std::vector<T> >
    : public InheritIf<GenericVectorCallbackOutputTraits<T>,
                       !IsBaseOf<Resource, T>::value>,
      public InheritIf<ResourceVectorCallbackOutputTraits<T>,
                       IsBaseOf<Resource, T>::value> {
};

// A specialization of CallbackOutputTraits to provide the callback system
// the information on how to handle vectors of pp::Var. Vectors of resources
// and plain data are handled separately. See the above definition for more.
template<>
struct CallbackOutputTraits< std::vector<pp::Var> > {
  // All arrays are output via a PP_ArrayOutput type.
  typedef PP_ArrayOutput* APIArgType;

  // We store the array as this adapter which combines the PP_ArrayOutput
  // structure with the underlying std::vector that it will write into.
  typedef VarArrayOutputAdapterWithStorage StorageType;

  // Retrieves the PP_ArrayOutput interface for our vector object that the
  // browser will use to write into our code.
  static inline APIArgType StorageToAPIArg(StorageType& t) {
    return t.pp_array_output();
  }

  // Retrieves the underlying vector that can be passed to the plugin.
  static inline std::vector<pp::Var>& StorageToPluginArg(StorageType& t) {
    return t.output();
  }
};

}  // namespace internal
}  // namespace pp

#endif  // PPAPI_CPP_OUTPUT_TRAITS_H_
