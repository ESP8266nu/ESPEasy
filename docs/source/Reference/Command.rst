.. include:: ../Plugin/_plugin_substitutions.repl

Command Reference
*****************

ESP Easy offers a set of commands to control hardware devices and provide some basic local control using rules. There are several ways to launch commands on ESP Easy:

.. csv-table::
  :header: "Protocol", "Syntax", "Extra information"
  :widths: 8, 15, 15

  "
  HTTP
  ","
  **http://<espeasyip>/control?cmd=** ``<command>``
  ","
  Send commands over the HTTP protocol.
  "
  "
  MQTT
  ","
  **<MQTT subscribe template>/cmd** with payload: ``<command>``
  ","
  Send commands over the MQTT protocol.
  "
  "
  Serial (TTL)
  ","
  ``<command>``
  ","
  Send commands using serial (RX/TX). Just type the ``<command>``
  "
  "
  UDP
  ","
  **SendTo,<unit nr>,** ``<command>``
  ","
  Send commands from one ESP Easy unit to another. Setup UDP ESP Easy peer-2-peer controller first.
  "
  "
  Rules
  ","
  ``<command>``
  ","
  Internally within ESP Easy. Just enter the ``<command>`` within an event block or conditional block.
  "

Commands are divided into several classes:

:red:`Internal` can be run from serial and rules engine

:green:`Rules` can be run from serial and rules engine

:cyan:`Plugin` can be run from serial, rules engine, HTTP, MQTT

:blue:`Special` can be used from any source

Command a specific task for multiple instances of a plugin
----------------------------------------------------------

When multiple tasks are assigned to run the same plugin, one may want to address a specific task to run the command.

In order to do so, prefix the command with the intended task name.
Either by using ``[<TaskName>].`` or ``[<TaskNumber>].`` (square brackets are optional) to address a specific instance. 

N.B. This requires the plugin names to be unique (if the TaskName variant is used).

Examples:

``[Display1].oledframedcmd,3,'Hello World'``

``[Display2].oledframedcmd,4,'From the other side'``

This will display 'Hello World' on the 3rd line of the display with name 'Display1', and 'From the other side' on line 4 of the display named 'Display2'.


``[AC1].irsendac,{<some_json_to_control_AC>}``

``[AC2].irsendac,{<some_json_to_control_AC>}``

This allows to control multiple IR controlled AC's from one ESP.


Internal Commands
-----------------

Commands handled by ESPEasy core.
These are not part of a plugin.

.. include:: ../Plugin/P000_commands.repl


GPIO Commands
-------------

Internal GPIO
~~~~~~~~~~~~~

.. include:: ../Plugin/P001_commands_GPIO.repl

External MCPGPIO
~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P009_commands.repl

External PCFGPIO
~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P019_commands.repl


Ringtone Internal GPIO
~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P001_commands_RTTTL.repl


Plugin based commands
---------------------

Besides the internal commands there's also plugin specific commands. 

These can only be handled when the specific plugin is included in the ESPEasy build (and the plugin is assigned to an enabled task)

.. P001 :ref:`P001_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P001_commands.repl


.. P002 :ref:`P002_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P002_commands.repl


P003 :ref:`P003_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P003_commands.repl


.. P004 :ref:`P004_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P004_commands.repl


.. P005 :ref:`P005_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P005_commands.repl


.. P006 :ref:`P006_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P006_commands.repl


.. P007 :ref:`P007_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P007_commands.repl


.. P008 :ref:`P008_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P008_commands.repl


.. P009 :ref:`P009_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P009_commands.repl


.. P010 :ref:`P010_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P010_commands.repl


.. P011 :ref:`P011_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P011_commands.repl


P012 :ref:`P012_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P012_commands.repl


.. P013 :ref:`P013_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P013_commands.repl


.. P014 :ref:`P014_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P014_commands.repl


.. P015 :ref:`P015_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P015_commands.repl


.. P016 :ref:`P016_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P016_commands.repl


.. P017 :ref:`P017_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P017_commands.repl


.. P018 :ref:`P018_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P018_commands.repl


.. P019 :ref:`P019_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P019_commands.repl


.. P020 :ref:`P020_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P020_commands.repl


.. P021 :ref:`P021_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P021_commands.repl


.. P022 :ref:`P022_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P022_commands.repl


.. P023 :ref:`P023_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P023_commands.repl


.. P024 :ref:`P024_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P024_commands.repl


.. P025 :ref:`P025_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P025_commands.repl


.. P026 :ref:`P026_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P026_commands.repl


.. P027 :ref:`P027_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P027_commands.repl


.. P028 :ref:`P028_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P028_commands.repl


.. P029 :ref:`P029_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P029_commands.repl


.. P030 :ref:`P030_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P030_commands.repl


.. P031 :ref:`P031_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P031_commands.repl


.. P032 :ref:`P032_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P032_commands.repl


.. P033 :ref:`P033_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P033_commands.repl


.. P034 :ref:`P034_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P034_commands.repl


.. P035 :ref:`P035_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P035_commands.repl


P036 :ref:`P036_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P036_commands.repl


.. P037 :ref:`P037_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P037_commands.repl


.. P038 :ref:`P038_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P038_commands.repl


.. P039 :ref:`P039_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P039_commands.repl


.. P040 :ref:`P040_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P040_commands.repl


.. P041 :ref:`P041_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P041_commands.repl


.. P042 :ref:`P042_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P042_commands.repl


.. P043 :ref:`P043_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P043_commands.repl


.. P044 :ref:`P044_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P044_commands.repl


.. P045 :ref:`P045_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P045_commands.repl


.. P046 :ref:`P046_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P046_commands.repl


.. P047 :ref:`P047_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P047_commands.repl


P048 :ref:`P048_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P048_commands.repl


.. P049 :ref:`P049_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P049_commands.repl


.. P050 :ref:`P050_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P050_commands.repl


.. P051 :ref:`P051_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P051_commands.repl


P052 :ref:`P052_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P052_commands.repl


.. P053 :ref:`P053_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P053_commands.repl


.. P054 :ref:`P054_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P054_commands.repl


.. P055 :ref:`P055_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P055_commands.repl


.. P056 :ref:`P056_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P056_commands.repl


.. P057 :ref:`P057_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P057_commands.repl


.. P058 :ref:`P058_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P058_commands.repl


.. P059 :ref:`P059_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P059_commands.repl


.. P060 :ref:`P060_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P060_commands.repl


.. P061 :ref:`P061_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P061_commands.repl


.. P062 :ref:`P062_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P062_commands.repl


.. P063 :ref:`P063_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P063_commands.repl


.. P064 :ref:`P064_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P064_commands.repl


P065 :ref:`P065_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P065_commands.repl


.. P066 :ref:`P066_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P066_commands.repl


.. P067 :ref:`P067_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P067_commands.repl


.. P068 :ref:`P068_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P068_commands.repl


.. P069 :ref:`P069_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P069_commands.repl


.. P070 :ref:`P070_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P070_commands.repl


.. P071 :ref:`P071_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P071_commands.repl


.. P072 :ref:`P072_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P072_commands.repl


.. P073 :ref:`P073_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P073_commands.repl


.. P074 :ref:`P074_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P074_commands.repl


P075 :ref:`P075_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P075_commands.repl


P076 :ref:`P076_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P076_commands.repl


.. P077 :ref:`P077_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P077_commands.repl


.. P078 :ref:`P078_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P078_commands.repl


P079 :ref:`P079_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P079_commands.repl


.. P080 :ref:`P080_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P080_commands.repl


.. P081 :ref:`P081_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P081_commands.repl


.. P082 :ref:`P082_page`
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. .. include:: ../Plugin/P082_commands.repl

P088 :ref:`P088_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P088_commands.repl


P091 :ref:`P091_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P091_commands.repl


P093 :ref:`P093_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P093_commands.repl

P094 :ref:`P094_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P094_commands.repl

P095 :ref:`P095_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P095_commands.repl

P099 :ref:`P099_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P099_commands.repl


P101 :ref:`P101_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P101_commands.repl



P115 :ref:`P115_page`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Plugin/P115_commands.repl
