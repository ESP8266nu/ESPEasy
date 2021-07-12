.. include:: ../Plugin/_plugin_substitutions.repl
.. include:: _plugin_categories.repl

#######
Plugins
#######

Similar to a smartphones "apps" the ESP Easy plugins extends the functionality of
the core operating system. You can use as many plugins as you available tasks.

List of official plugins
========================

There are different released versions of ESP Easy:

:green:`NORMAL` is the stable release, you can consider these plugins reliable and you can use these in production.

:yellow:`TESTING` (split into A/B/C/D sets) with new plugins that have not yet been fully tested and proven stable. Because of space limitations, this is split into 4 sets. When only TESTING is mentioned, the plugin is available in all TESTING builds.

.. comment :red:`DEVELOPMENT` is used for plugins that are still being developed and are not considered stable at all.

:yellow:`ENERGY` :yellow:`DISPLAY` are specialized builds holding all Energy- and Display- related plugins.

:yellow:`MAX` is the build that has all plugins that are available in the ESPEasy repository. Only available for ESP32 16MB Flash units.

.. csv-table::
   :header: "Plugin name", "Build set", "Plugin number"
   :widths: 10, 8, 5

   ":ref:`P000_page`","|P000_status|","P000 (core)"
   ":ref:`P001_page`","|P001_status|","P001"
   ":ref:`P002_page`","|P002_status|","P002"
   ":ref:`P003_page`","|P003_status|","P003"
   ":ref:`P004_page`","|P004_status|","P004"
   ":ref:`P005_page`","|P005_status|","P005"
   ":ref:`P006_page`","|P006_status|","P006"
   ":ref:`P007_page`","|P007_status|","P007"
   ":ref:`P008_page`","|P008_status|","P008"
   ":ref:`P009_page`","|P009_status|","P009"
   ":ref:`P010_page`","|P010_status|","P010"
   ":ref:`P011_page`","|P011_status|","P011"
   ":ref:`P012_page`","|P012_status|","P012"
   ":ref:`P013_page`","|P013_status|","P013"
   ":ref:`P014_page`","|P014_status|","P014"
   ":ref:`P015_page`","|P015_status|","P015"
   ":ref:`P016_page`","|P016_status|","P016"
   ":ref:`P017_page`","|P017_status|","P017"
   ":ref:`P018_page`","|P018_status|","P018"
   ":ref:`P019_page`","|P019_status|","P019"
   ":ref:`P020_page`","|P020_status|","P020"
   ":ref:`P021_page`","|P021_status|","P021"
   ":ref:`P022_page`","|P022_status|","P022"
   ":ref:`P023_page`","|P023_status|","P023"
   ":ref:`P024_page`","|P024_status|","P024"
   ":ref:`P025_page`","|P025_status|","P025"
   ":ref:`P026_page`","|P026_status|","P026"
   ":ref:`P027_page`","|P027_status|","P027"
   ":ref:`P028_page`","|P028_status|","P028"
   ":ref:`P029_page`","|P029_status|","P029"
   ":ref:`P030_page`","|P030_status|","P030"
   ":ref:`P031_page`","|P031_status|","P031"
   ":ref:`P032_page`","|P032_status|","P032"
   ":ref:`P033_page`","|P033_status|","P033"
   ":ref:`P034_page`","|P034_status|","P034"
   ":ref:`P035_page`","|P035_status|","P035"
   ":ref:`P036_page`","|P036_status|","P036"
   ":ref:`P037_page`","|P037_status|","P037"
   ":ref:`P038_page`","|P038_status|","P038"
   ":ref:`P039_page`","|P039_status|","P039"
   ":ref:`P040_page`","|P040_status|","P040"
   ":ref:`P041_page`","|P041_status|","P041"
   ":ref:`P042_page`","|P042_status|","P042"
   ":ref:`P043_page`","|P043_status|","P043"
   ":ref:`P044_page`","|P044_status|","P044"
   ":ref:`P045_page`","|P045_status|","P045"
   ":ref:`P046_page`","|P046_status|","P046"
   ":ref:`P047_page`","|P047_status|","P047"
   ":ref:`P048_page`","|P048_status|","P048"
   ":ref:`P049_page`","|P049_status|","P049"
   ":ref:`P050_page`","|P050_status|","P050"
   ":ref:`P051_page`","|P051_status|","P051"
   ":ref:`P052_page`","|P052_status|","P052"
   ":ref:`P053_page`","|P053_status|","P053"
   ":ref:`P054_page`","|P054_status|","P054"
   ":ref:`P055_page`","|P055_status|","P055"
   ":ref:`P056_page`","|P056_status|","P056"
   ":ref:`P057_page`","|P057_status|","P057"
   ":ref:`P058_page`","|P058_status|","P058"
   ":ref:`P059_page`","|P059_status|","P059"
   ":ref:`P060_page`","|P060_status|","P060"
   ":ref:`P061_page`","|P061_status|","P061"
   ":ref:`P062_page`","|P062_status|","P062"
   ":ref:`P063_page`","|P063_status|","P063"
   ":ref:`P064_page`","|P064_status|","P064"
   ":ref:`P065_page`","|P065_status|","P065"
   ":ref:`P066_page`","|P066_status|","P066"
   ":ref:`P067_page`","|P067_status|","P067"
   ":ref:`P068_page`","|P068_status|","P068"
   ":ref:`P069_page`","|P069_status|","P069"
   ":ref:`P070_page`","|P070_status|","P070"
   ":ref:`P071_page`","|P071_status|","P071"
   ":ref:`P072_page`","|P072_status|","P072"
   ":ref:`P073_page`","|P073_status|","P073"
   ":ref:`P074_page`","|P074_status|","P074"
   ":ref:`P075_page`","|P075_status|","P075"
   ":ref:`P076_page`","|P076_status|","P076"
   ":ref:`P077_page`","|P077_status|","P077"
   ":ref:`P078_page`","|P078_status|","P078"
   ":ref:`P079_page`","|P079_status|","P079"
   ":ref:`P080_page`","|P080_status|","P080"
   ":ref:`P081_page`","|P081_status|","P081"
   ":ref:`P082_page`","|P082_status|","P082"
   ":ref:`P083_page`","|P083_status|","P083"
   ":ref:`P084_page`","|P084_status|","P084"
   ":ref:`P085_page`","|P085_status|","P085"
   ":ref:`P086_page`","|P086_status|","P086"
   ":ref:`P087_page`","|P087_status|","P087"
   ":ref:`P088_page`","|P088_status|","P088"
   ":ref:`P089_page`","|P089_status|","P089"
   ":ref:`P090_page`","|P090_status|","P090"
   ":ref:`P091_page`","|P091_status|","P091"
   ":ref:`P092_page`","|P092_status|","P092"
   ":ref:`P093_page`","|P093_status|","P093"
   ":ref:`P094_page`","|P094_status|","P094"
   ":ref:`P095_page`","|P095_status|","P095"
   ":ref:`P097_page`","|P097_status|","P097"
   ":ref:`P099_page`","|P099_status|","P099"
   ":ref:`P100_page`","|P100_status|","P100"
   ":ref:`P101_page`","|P101_status|","P101"
   ":ref:`P102_page`","|P102_status|","P102"
   ":ref:`P103_page`","|P103_status|","P103"
   ":ref:`P104_page`","|P104_status|","P104"
   ":ref:`P105_page`","|P105_status|","P105"
   ":ref:`P106_page`","|P106_status|","P106"
   ":ref:`P107_page`","|P107_status|","P107"
   ":ref:`P108_page`","|P108_status|","P108"
   ":ref:`P109_page`","|P109_status|","P109"
   ":ref:`P110_page`","|P110_status|","P110"
   ":ref:`P111_page`","|P111_status|","P111"
   ":ref:`P113_page`","|P113_status|","P113"
   ":ref:`P114_page`","|P114_status|","P114"
   ":ref:`P115_page`","|P115_status|","P115"


Internal GPIO handling
----------------------

Plugins: :ref:`P000_page`

Hardware: |P000_usedby_GPIO|, |P000_usedby_RTTTL|, |P000_usedby_Relay|, |P000_usedby_Servo|, |P000_usedby_LevelConverter|

Analog input
------------

Plugins: |Plugin_Analog_input|

Communication
-------------

Plugins: |Plugin_Communication|

Display
-------

Plugins: |Plugin_Display|

Distance
--------

Plugins: |Plugin_Distance|

Dust
----

Plugins: |Plugin_Dust|

Energy (AC)
-----------

Plugins: |Plugin_Energy_AC|

Energy (DC)
-----------

Plugins: |Plugin_Energy_DC|

Energy (Heat)
-------------

Plugins: |Plugin_Energy_Heat|

Environment
-----------

Plugins: |Plugin_Environment|

Hardware: |P004_usedby|, |P005_usedby|, |P006_usedby|, |P024_usedby|, |P028_usedby|, |P030_usedby|

Extra IO
--------

Plugins: |Plugin_Extra_IO|

Gases
-----

Plugins: |Plugin_Gases|

Hardware: |P052_usedby|

Generic
-------

Plugins: |Plugin_Generic|

Hardware: |P003_usedby|

Gesture
-------

Plugins: |Plugin_Gesture|

Gyro
----

Plugins: |Plugin_Gyro|

Hardware
--------

Plugins: |Plugin_Hardware|

Hardware: |P046_usedby|

Keypad
------

Plugins: |Plugin_Keypad|

Light/Color
-----------

Plugins: |Plugin_Light_Color|

Light/Lux
---------

Plugins: |Plugin_Light_Lux|

Hardware: |P015_usedby|

Light/UV
-----------

Plugins: |Plugin_Light_UV|

Motor
-----

Plugins: |Plugin_Motor|

Notify
------

Plugins: |Plugin_Notify|

Output
------

Plugins: |Plugin_Output|

Position
--------

Plugins: |Plugin_Position|

Hardware: |P013_usedby|, |P082_usedby|

Regulator
---------

Plugins: |Plugin_Regulator|

RFID
----

Plugins: |Plugin_RFID|

Switch input
------------

Plugins: |Plugin_Switch_input|

Hardware: |P001_usedby|

Touch
-----

Plugins: |Plugin_Touch|

Weight
------

Plugins: |Plugin_Weight|