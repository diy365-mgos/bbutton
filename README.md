# bButtons Library
## Overview
A bButton is a specilized version of [bSensor](https://github.com/diy365-mgos/bsensor). It allows you to easily manage bSensors having just binary (true/false) state.
## Features
- **Universal value-types** - You can read any sensor value within the supported [bVariant](https://github.com/diy365-mgos/bvar) data-types.
- **Verbose state** - Optionally, you can configure verbose states intead of default booleans (e.g: 'ON'/'OFF').
- **GPIO as sensors** - You can easily manage GPIO as sensors. Just include the [bThings GPIO library](https://github.com/diy365-mgos/bthing-gpio) in your project.
## Get Started in C/C++
Include the library in your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/diy365-mgos/bbutton
```
#### Example 1 - Push button
Create a bButton for reading a *pressed/released* button state. Before running the code sample, you must wire your boardas indicated in the schema below.

![Example 1 - schema](docs/example_1_schema.png)

In addition, include this library in your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/diy365-mgos/bthing-gpio
```
```c
```
## C/C++ APIs Reference
### Inherited APIs
A bButton inherits APIs from:
- [bThing](https://github.com/diy365-mgos/bthing)
- [bSensor](https://github.com/diy365-mgos/bsensor)
### MGOS_BBUTTON_TYPE
```c
#define MGOS_BBUTTON_TYPE 16
```
The bButton type ID returned by [mgos_bthing_get_type()](https://github.com/diy365-mgos/bthing#mgos_bthing_get_type). It can be used with [mgos_bthing_is_typeof()](https://github.com/diy365-mgos/bthing#mgos_bthing_is_typeof).

Example:
```c
mgos_bbutton_t btn = mgos_bbutton_create(...);
if (mgos_bthing_is_typeof(MGOS_BBUTTON_THINGCAST(btn), MGOS_BBUTTON_TYPE))
  LOG(LL_INFO, ("I'm a bButton."));
if (mgos_bthing_is_typeof(MGOS_BBUTTON_THINGCAST(btn), MGOS_BSENSOR_TYPE))
  LOG(LL_INFO, ("I'm a bSensor."));
if (mgos_bthing_is_typeof(MGOS_BBUTTON_THINGCAST(btn), MGOS_BTHING_TYPE_SENSOR))
  LOG(LL_INFO, ("I'm a bThing sensor."));
```
Output console:
```bash
I'm a bButton.
I'm a bSensor.
I'm a bThing sensor.
```
### MGOS_BBUTTON_THINGCAST
```c
mgos_bthing_t MGOS_BBUTTON_THINGCAST(mgos_bbutton_t button);
```
Casts a bButton to a generic bThing to be used with [inherited bThing APIs](https://github.com/diy365-mgos/bthing).

|Parameter||
|--|--|
|button|A bButton.|

Example:
```c
mgos_bbutton_t btn = mgos_bbutton_create(...);
LOG(LL_INFO, ("%s successfully created.", mgos_bthing_get_id(MGOS_BBUTTON_THINGCAST(btn)));
```
### MGOS_BBUTTON_DOWNCAST
```c
mgos_bsensor_t MGOS_BBUTTON_DOWNCAST(mgos_bbutton_t button);
```
Casts a bButton to a bSensor to be used with [inherited bSensor APIs](https://github.com/diy365-mgos/bsensor).

|Parameter||
|--|--|
|button|A bButton.|
### mgos_bbutton_create
```c
mgos_bbutton_t mgos_bbutton_create(const char *id, enum mgos_bthing_pub_state_mode pub_state_mode);
```
Creates a bButton. Returns `NULL` on error.

|Parameter||
|--|--|
|id|The bButton ID.|
|pub_state_mode|The [publish-state mode](https://github.com/diy365-mgos/bthing#enum-mgos_bthing_pub_state_mode). The `MGOS_BTHING_PUB_STATE_MODE_CHANGED` value is not allowed.|
## To Do
- Implement javascript APIs for [Mongoose OS MJS](https://github.com/mongoose-os-libs/mjs).