////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020 Realm Inc.
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

#import <Foundation/Foundation.h>

#import <Realm/RLMObjectBase.h>
#import <Realm/RLMThreadSafeReference.h>

NS_ASSUME_NONNULL_BEGIN

@class RLMObjectSchema, RLMPropertyDescriptor, RLMRealm, RLMNotificationToken, RLMPropertyChange;

/**
 `RLMEmbeddedObject` is a base class for model objects representing data stored in Realms.

 Define your model classes by subclassing `RLMEmbeddedObject` and adding properties to be managed.
 Then instantiate and use your custom subclasses instead of using the `RLMEmbeddedObject` class directly.

     // Dog.h
     @interface Dog : RLMEmbeddedObject
     @property NSString *name;
     @property BOOL      adopted;
     @end

     // Dog.m
     @implementation Dog
     @end //none needed

 ### Supported property types

 - `NSString`
 - `NSInteger`, `int`, `long`, `float`, and `double`
 - `BOOL` or `bool`
 - `NSDate`
 - `NSData`
 - `NSNumber<X>`, where `X` is one of `RLMInt`, `RLMFloat`, `RLMDouble` or `RLMBool`, for optional number properties
 - `RLMEmbeddedObject` subclasses, to model many-to-one relationships.
 - `RLMArray<X>`, where `X` is an `RLMEmbeddedObject` subclass, to model many-to-many relationships.

 ### Querying

 You can initiate queries directly via the class methods: `allObjects`, `objectsWhere:`, and `objectsWithPredicate:`.
 These methods allow you to easily query a custom subclass for instances of that class in the default Realm.

 To search in a Realm other than the default Realm, use the `allObjectsInRealm:`, `objectsInRealm:where:`,
 and `objectsInRealm:withPredicate:` class methods.

 @see `RLMRealm`

 ### Relationships

 See our [Cocoa guide](https://realm.io/docs/objc/latest#relationships) for more details.

 ### Key-Value Observing

 All `RLMEmbeddedObject` properties (including properties you create in subclasses) are
 [Key-Value Observing compliant](https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/KeyValueObserving/KeyValueObserving.html),
 except for `realm` and `objectSchema`.

 Keep the following tips in mind when observing Realm objects:

 1. Unlike `NSMutableArray` properties, `RLMArray` properties do not require
    using the proxy object returned from `-mutableArrayValueForKey:`, or defining
    KVC mutation methods on the containing class. You can simply call methods on
    the `RLMArray` directly; any changes will be automatically observed by the containing
    object.
 2. Unmanaged `RLMEmbeddedObject` instances cannot be added to a Realm while they have any
    observed properties.
 3. Modifying managed `RLMEmbeddedObject`s within `-observeValueForKeyPath:ofObject:change:context:`
    is not recommended. Properties may change even when the Realm is not in a write
    transaction (for example, when `-[RLMRealm refresh]` is called after changes
    are made on a different thread), and notifications sent prior to the change
    being applied (when `NSKeyValueObservingOptionPrior` is used) may be sent at
    times when you *cannot* begin a write transaction.
 */

@interface RLMEmbeddedObject : RLMObjectBase <RLMThreadConfined>

#pragma mark - Creating & Initializing Objects

/**
 Creates an unmanaged instance of a Realm object.

 Call `addObject:` on an `RLMRealm` instance to add an unmanaged object into that Realm.

 @see `[RLMRealm addObject:]`
 */
- (instancetype)init NS_DESIGNATED_INITIALIZER;


/**
 Creates an unmanaged instance of a Realm object.

 Pass in an `NSArray` or `NSDictionary` instance to set the values of the object's properties.

 Call `addObject:` on an `RLMRealm` instance to add an unmanaged object into that Realm.

 @see `[RLMRealm addObject:]`
 */
- (instancetype)initWithValue:(id)value;


/**
 Returns the class name for a Realm object subclass.

 @warning Do not override. Realm relies on this method returning the exact class
          name.

 @return  The class name for the model class.
 */
+ (NSString *)className;

#pragma mark - Properties

/**
 The Realm which manages the object, or `nil` if the object is unmanaged.
 */
@property (nonatomic, readonly, nullable) RLMRealm *realm;

/**
 The object schema which lists the managed properties for the object.
 */
@property (nonatomic, readonly) RLMObjectSchema *objectSchema;

/**
 Indicates if the object can no longer be accessed because it is now invalid.

 An object can no longer be accessed if the object has been deleted from the Realm that manages it, or
 if `invalidate` is called on that Realm.
 */
@property (nonatomic, readonly, getter = isInvalidated) BOOL invalidated;

/**
 Indicates if this object is frozen.

 @see `-[RLMEmbeddedObject freeze]`
 */
@property (nonatomic, readonly, getter = isFrozen) BOOL frozen;


#pragma mark - Customizing your Objects

/**
 Returns an array of property names for properties which should be indexed.

 Only string, integer, boolean, and `NSDate` properties are supported.

 @return    An array of property names.
 */
+ (NSArray<NSString *> *)indexedProperties;

/**
 Override this method to specify the default values to be used for each property.

 @return    A dictionary mapping property names to their default values.
 */
+ (nullable NSDictionary *)defaultPropertyValues;

/**
 Override this method to specify the names of properties to ignore. These properties will not be managed by the Realm
 that manages the object.

 @return    An array of property names to ignore.
 */
+ (nullable NSArray<NSString *> *)ignoredProperties;

/**
 Override this method to specify the names of properties that are non-optional (i.e. cannot be assigned a `nil` value).

 By default, all properties of a type whose values can be set to `nil` are considered optional properties.
 To require that an object in a Realm always store a non-`nil` value for a property,
 add the name of the property to the array returned from this method.

 Properties of `RLMEmbeddedObject` type cannot be non-optional. Array and `NSNumber` properties
 can be non-optional, but there is no reason to do so: arrays do not support storing nil, and
 if you want a non-optional number you should instead use the primitive type.

 @return    An array of property names that are required.
 */
+ (NSArray<NSString *> *)requiredProperties;

/**
 Override this method to provide information related to properties containing linking objects.

 Each property of type `RLMLinkingObjects` must have a key in the dictionary returned by this method consisting
 of the property name. The corresponding value must be an instance of `RLMPropertyDescriptor` that describes the class
 and property that the property is linked to.

     return @{ @"owners": [RLMPropertyDescriptor descriptorWithClass:Owner.class propertyName:@"dogs"] };

 @return     A dictionary mapping property names to `RLMPropertyDescriptor` instances.
 */
+ (NSDictionary<NSString *, RLMPropertyDescriptor *> *)linkingObjectsProperties;

#pragma mark - Notifications

/**
 A callback block for `RLMEmbeddedObject` notifications.

 If the object is deleted from the managing Realm, the block is called with
 `deleted` set to `YES` and the other two arguments are `nil`. The block will
 never be called again after this.

 If the object is modified, the block will be called with `deleted` set to
 `NO`, a `nil` error, and an array of `RLMPropertyChange` objects which
 indicate which properties of the objects were modified.

 If an error occurs, `deleted` will be `NO`, `changes` will be `nil`, and
 `error` will include information about the error. The block will never be
 called again after an error occurs.
 */
typedef void (^RLMObjectChangeBlock)(BOOL deleted,
                                     NSArray<RLMPropertyChange *> *_Nullable changes,
                                     NSError *_Nullable error);

/**
 Registers a block to be called each time the object changes.

 The block will be asynchronously called after each write transaction which
 deletes the object or modifies any of the managed properties of the object,
 including self-assignments that set a property to its existing value.

 For write transactions performed on different threads or in differen
 processes, the block will be called when the managing Realm is
 (auto)refreshed to a version including the changes, while for local write
 transactions it will be called at some point in the future after the write
 transaction is committed.

 Notifications are delivered via the standard run loop, and so can't be
 delivered while the run loop is blocked by other activity. When notifications
 can't be delivered instantly, multiple notifications may be coalesced into a
 single notification.

 Unlike with `RLMArray` and `RLMResults`, there is no "initial" callback made
 after you add a new notification block.

 Only objects which are managed by a Realm can be observed in this way. You
 must retain the returned token for as long as you want updates to be sent to
 the block. To stop receiving updates, call `-invalidate` on the token.

 It is safe to capture a strong reference to the observed object within the
 callback block. There is no retain cycle due to that the callback is retained
 by the returned token and not by the object itself.

 @warning This method cannot be called during a write transaction, when the
          containing Realm is read-only, or on an unmanaged object.

 @param block The block to be called whenever a change occurs.
 @return A token which must be held for as long as you want updates to be delivered.
 */
- (RLMNotificationToken *)addNotificationBlock:(RLMObjectChangeBlock)block;

#pragma mark - Other Instance Methods

/**
 Returns YES if another Realm object instance points to the same object as the receiver in the Realm managing
 the receiver.

 For frozen objects and object types with a primary key, `isEqual:` is
 overridden to use the same logic as this method (along with a corresponding
 implementation for `hash`). Non-frozen objects without primary keys use
 pointer identity for `isEqual:` and `hash`.

 @param object  The object to compare the receiver to.

 @return    Whether the object represents the same object as the receiver.
 */
- (BOOL)isEqualToObject:(RLMEmbeddedObject *)object;

/**
 Returns a frozen (immutable) snapshot of this object.

 The frozen copy is an immutable object which contains the same data as this
 object currently contains, but will not update when writes are made to the
 containing Realm. Unlike live objects, frozen objects can be accessed from any
 thread.

 - warning: Holding onto a frozen object for an extended period while performing write
 transaction on the Realm may result in the Realm file growing to large sizes. See
 `Realm.Configuration.maximumNumberOfActiveVersions` for more information.
 - warning: This method can only be called on a managed object.
 */
- (instancetype)freeze NS_RETURNS_RETAINED;

#pragma mark - Dynamic Accessors

/// :nodoc:
- (nullable id)objectForKeyedSubscript:(NSString *)key;

/// :nodoc:
- (void)setObject:(nullable id)obj forKeyedSubscript:(NSString *)key;

@end

#pragma mark - RLMArray Property Declaration

/**
 Properties on `RLMEmbeddedObject`s of type `RLMArray` must have an associated type. A type is associated
 with an `RLMArray` property by defining a protocol for the object type that the array should contain.
 To define the protocol for an object, you can use the macro RLM_ARRAY_TYPE:

     RLM_ARRAY_TYPE(ObjectType)
     ...
     @property RLMArray<ObjectType *><ObjectType> *arrayOfObjectTypes;
  */
#define RLM_ARRAY_TYPE(RLM_OBJECT_SUBCLASS)\
@protocol RLM_OBJECT_SUBCLASS <NSObject>   \
@end

NS_ASSUME_NONNULL_END
