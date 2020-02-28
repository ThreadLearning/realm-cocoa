////////////////////////////////////////////////////////////////////////////
//
// Copyright 2014 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#import "RLMEmbeddedObject.h"
#import "RLMObject_Private.hpp"

#import "RLMAccessor.h"
#import "RLMArray.h"
#import "RLMCollection_Private.hpp"
#import "RLMObjectBase_Private.h"
#import "RLMObjectSchema_Private.hpp"
#import "RLMObjectStore.h"
#import "RLMProperty_Private.h"
#import "RLMQueryUtil.hpp"
#import "RLMRealm_Private.hpp"
#import "RLMSchema_Private.h"

#import "collection_notifications.hpp"
#import "object.hpp"

// We declare things in RLMEmbeddedObject which are actually implemented in RLMObjectBase
// for documentation's sake, which leads to -Wunimplemented-method warnings.
// Other alternatives to this would be to disable -Wunimplemented-method for this
// file (but then we could miss legitimately missing things), or declaring the
// inherited things in a category (but they currently aren't nicely grouped for
// that).
@implementation RLMEmbeddedObject

// synthesized in RLMObjectBase
@dynamic invalidated, realm, objectSchema;

#pragma mark - Designated Initializers

- (instancetype)init {
    return [super init];
}

#pragma mark - Convenience Initializers

- (instancetype)initWithValue:(id)value {
    if (!(self = [self init])) {
        return nil;
    }
    RLMInitializeWithValue(self, value, RLMSchema.partialPrivateSharedSchema);
    return self;
}

#pragma mark - Subscripting

- (id)objectForKeyedSubscript:(NSString *)key {
    return RLMObjectBaseObjectForKeyedSubscript(self, key);
}

- (void)setObject:(id)obj forKeyedSubscript:(NSString *)key {
    RLMObjectBaseSetObjectForKeyedSubscript(self, key, obj);
}

#pragma mark - Other Instance Methods

- (BOOL)isEqualToObject:(RLMObjectBase *)object {
    return [object isKindOfClass:RLMObjectBase.class] && RLMObjectBaseAreEqual(self, object);
}

- (instancetype)freeze {
    return RLMObjectFreeze(self);
}

- (RLMNotificationToken *)addNotificationBlock:(RLMObjectChangeBlock)block {
    return RLMObjectAddNotificationBlock(self, block);
}

+ (NSString *)className {
    return [super className];
}

#pragma mark - Default values for schema definition

+ (NSString *)primaryKey {
    return nil;
}

+ (NSArray *)indexedProperties {
    return @[];
}

+ (NSDictionary *)linkingObjectsProperties {
    return @{};
}

+ (NSDictionary *)defaultPropertyValues {
    return nil;
}

+ (NSArray *)ignoredProperties {
    return nil;
}

+ (NSArray *)requiredProperties {
    return @[];
}

+ (bool)_realmIgnoreClass {
    return false;
}

+ (bool)isEmbedded {
    return true;
}
@end
