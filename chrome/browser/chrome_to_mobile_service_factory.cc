// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chrome_to_mobile_service_factory.h"

#include "chrome/browser/chrome_to_mobile_service.h"
#include "chrome/browser/profiles/profile_dependency_manager.h"
#include "chrome/browser/signin/token_service_factory.h"

// static
ChromeToMobileServiceFactory* ChromeToMobileServiceFactory::GetInstance() {
  return Singleton<ChromeToMobileServiceFactory>::get();
}

// static
ChromeToMobileService* ChromeToMobileServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<ChromeToMobileService*>(
    GetInstance()->GetServiceForProfile(profile, true));
}

ProfileKeyedService* ChromeToMobileServiceFactory::BuildServiceInstanceFor(
    Profile* profile) const {
  // Ensure that the service is not instantiated or used if it is disabled.
  if (!ChromeToMobileService::IsChromeToMobileEnabled())
    return NULL;

  return new ChromeToMobileService(profile);
}

ChromeToMobileServiceFactory::ChromeToMobileServiceFactory()
    : ProfileKeyedServiceFactory("ChromeToMobileService",
                                 ProfileDependencyManager::GetInstance()) {
  DependsOn(TokenServiceFactory::GetInstance());
}

ChromeToMobileServiceFactory::~ChromeToMobileServiceFactory() {}
