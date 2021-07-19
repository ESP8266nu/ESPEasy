Rules
*****

Introduction
============

Along with ESP Easy R108, a new feature was enabled, named Rules.
Rules can be used to create very simple flows to control devices on your ESP.

Enable Rules
------------

To enable rules, :menuselection:`Tools --> Advanced` and check the Rules checkbox.

After clicking Submit, you will find a new page added. Here you can start
experimenting with Rules:

[ADD_GUI_PICTURE]

The example above shows an experiment with a LED, connected via a resistor of
1k to GPIO12 and to ground.

A virtual switch needs to be created in the "Devices" section to allow the
reading the state of the LED (on or off). The Device needs to be "Switch Input"
with the following settings:

* Device Name - E1SW1
* Enabled - Ticked
* Internal Pull-Up - Ticked
* 1st GPIO - GPIO-12(D6)
* Switch Type - Switch
* Switch Button Type - Normal Switch

After rebooting the ESP, the LED will start blinking 10 seconds on then 10 seconds off.

Enjoy.

Device name special considerations
----------------------------------

To make events work as expected during rules processing, these characters can't be used in any device name:

* Operators: ``-``, ``+``, ``/``, ``*``, ``^`` and ``%``
* Unary operator: ``!``
* Equals sign: ``=``
* Delimiters: ``,``, ``[``, ``]``, ``{``, ``}``, ``(``, ``)`` and ``<space>``
* Devicename - Value separator: ``#``

An errormessage will be shown if any of these characters is used.

Special Notations
-----------------

* ``[...#...]`` Referring to task variable
* ``%...%`` Referring to system variable or standard conversions.
* ``{...:...}`` Referring to String conversions
* Quotes (single, double or back quotes) Marking begin and end of a command parameter



Syntax
------

The syntax of a rule can be single line:

.. code-block:: none

   on <trigger> do <action> endon

or multi-line (need to be closed with an "endon"):

.. code-block:: none

   on <trigger> do
    <action>
    <action>
    <action>
   endon

IF/ELSE - IF/ELSEIF/ELSE
------------------------

Also simple if ... else ... statements are possible:

.. code-block:: none

   on <trigger> do
    if <test>
      <action>
      <action>
    else
     <action>
    endif
   endon

If the "else" part is not needed it can be removed:

.. code-block:: none

   on <trigger> do
    if <test>
      <action>
      <action>
    endif
   endon

Also more complex if ... elseif ... else statements are possible (multiple elseif's can be used)

.. code-block:: none

   on <trigger> do
    if <test1>
      <action>
      <action>
    elseif <test2>
      <action>
      <action>
    else
     <action>
    endif
   endon

Again, if the "else" part is not needed it can be removed:

.. code-block:: none

   on <trigger> do
    if <test1>
      <action>
      <action>
    elseif <test2>
      <action>
      <action>
    elseif <test3>
      <action>
      <action>
    endif
   endon

AND/OR
------

Only simple if/else was possible in older versions, there was this workaround
for the limitation of not being able to nest. An "event" can be called from a
"trigger". This possibility of nesting events is also limited , due to its
consumption of stack space (IRAM). Depending on plug-ins in use this might
lead to unpredictable, unreliable behavior, advice is not to exceed 3 levels
of nesting.

.. code-block:: none

   on <trigger> do
    if <test1>
      event,<EventName1>
    endif
   endon
   
   on <EventName1> do
    if <test2>
      <action>
    endif
   endon

As of mega-201803.. we have the possibility to use AND/OR:

.. code-block:: none

   on test do
     if [test#a]=0 or [test#b]=0 or [test#c]=0
      event,ok
     else
      event,not_ok
     endif
   endon
   
   on test2 do
     if [test#a]=1 and [test#b]=1 and [test#c]=1
      event,ok
     else
      event,not_ok
     endif
   endon
   
   on test3 do
     if [test#a]=1 and [test#b]=1 or [test#c]=0
      event,ok
     else
      event,not_ok
     endif
   endon
   
   on test4 do
     if [test#a]=0
      event,ok
     else
      event,not_ok
     endif
   endon

Up to two AND/OR can be used per if statement, that means that you can test
three float values and if the statement is true/false corresponding action will take place.

Trigger
-------

.. code-block:: none

  <trigger>

The trigger can be an device value being changed:

.. code-block:: none

  DeviceName#ValueName

Operator (inequality function)
------------------------------

Or a inequality function:

.. code-block:: none

 DeviceName#ValueName<inequality function><value>

Where the "inequality function" is a simple check:

.. code-block:: none

  equal (=) to
  less (<) than
  greater (>) than
  less or equal (<=) to
  greater or equal (>=) to
  not equal (!= or <>) to

   DeviceName#ValueName<<value>
   DeviceName#ValueName=<value>
   DeviceName#ValueName><value>
   DeviceName#ValueName>=<value>
   DeviceName#ValueName<=<value>
   DeviceName#ValueName!=<value>
   DeviceName#ValueName<><value>

(System) events
---------------

Some special cases are these system triggers which is triggered upon
boot/reboot/time/sleep etc. of the unit:

.. include:: ../Plugin/P000_events.repl

Test
----

.. code-block:: none

  <test>

As described in the trigger section the test is a check done by checking
if the DeviceName#ValueName is meeting a criteria:

.. code-block:: none

  [DeviceName#ValueName]<inequality function><value>

Where the value must be a float value with a dot as decimal sign. The
DeviceName#ValueName is closed by (square) brackets "[" and "]".

Action
------

.. code-block:: none

 <action>

The action can be any system command found in the [ADD_LINK].
Also plugin specific command are available as long as the plugin is in use.
In the case mentioned earlier we use a action to trigger multiple logics
tests (the "event" command).

Comment
-------

If you want you can add comments to any row in your rules code. Just
remember to add them after the code and always begin with "//":

.. code-block:: none

 on <trigger> do //If this happens then do that...
  if <test>
    <action>
    <action>
  else
   <action>
  endif //this is another comment
 endon

Referring values
----------------

Rules and some plugins can use references to other (dynamic) values within ESPeasy.

The syntax for referring other values is: ``[...#...]``
Sometimes it can be useful to have some extra options, each separated using a '#' like this: ``[...#...#...]``

Reference to a value of a specific task: ``[TaskName#ValueName]``

Referring a value using some pre-defined format: ``[TaskName#ValueName#transformation#justification]``


For example, there is a task named "bme280" which has a value named "temperature".

Its value can be referenced like this: ``[bme280#temperature]``.
This can be used in some plugins like the "OLED Framed" plugin to populate some lines on the display.
It can also be used in rules. Every occurance of this text will then be replaced by its value.


N.B. these references to task values only yield a value when the task is enabled and its value is valid.




Event value (%eventvalue%)
--------------------------

Rules engine specific:

%eventvalue% - substitutes the event value (everything that comes after
the '=' sign, up to four values are possible).

Sample rules section:

.. code-block:: none

 on remoteTimerControl do
   timerSet,1,%eventvalue%
 endon

Now send this command to the ESP:

.. code-block:: none

 http://<espeasyip>/control?cmd=event,remoteTimerControl=5

and it will set rules timer no 1 to 5 seconds. Using this technique you can
parse a value from an event to the rule engine.

It is possible to use multiple event values. Some system events generate multiple event values.

For example, the ``Rules#Timer`` event has 2 event values (since build 2020/08/12):

* ``%eventvalue1%`` has the timer number (1 ... max timer ID)
* ``%eventvalue2%`` has the loop count for loop timers (since build 2020/08/12)

.. note::
 'timerSet' is a rule command and cannot be run directly from a remote command.

If you want to check the transferred value within rules on the receiving ESP
(condition in if-statement), you will need to write the transferred value into
a Dummy device using the TaskValueSet command. It is then possible to check
the value of the Dummy device as condition in if-statement within rules.

Multiple event values:

.. code-block:: none

 on ToggleGPIO do
   GPIO,%eventvalue1%,%eventvalue2%
 endon

You could then use the command "ToggleGPIO" with dynamic GPIO numbers and state.

.. code-block:: none

 http://<espeasyip>/control?cmd=event,ToggleGPIO=12,1

Task value events
-----------------

Tasks also send out events when a read was successful.

There is a number of triggers for a task to perform a read:

* Periodical read. A task calls its own read function every <interval> number of seconds. (Setting per task)
* ``TaskRun`` command. A task can be forced to take a reading via a command. This can be sent from rules, HTTP calls, etc.
* Some task reschedule their own read calls right after the sensor is done collecting data. (e.g. the BME280)

Event per task value
^^^^^^^^^^^^^^^^^^^^

By default, an event is created per read task value.
For example a task called "bme" (using BMx280 plugin) may output upto 3 values:

* Temperature
* Humidity
* Pressure

This would then generate upto 3 events:

* ``bme#Temperature=21.12``
* ``bme#Humidity=49.23``
* ``bme#Pressure=1010.34``

Single event with all values
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

(Added: 2021-01-11)

Each task may be configured to combine all task values in a single event, by checking "Single event with all values".

This will create a single event with variable name "All" like this:

* ``bme#All=21.12,49.23,1010.34``

To access all event values in the rules:

.. code-block:: none

 on bme#All do
   LogEntry,"temp: %eventvalue1% hum: %eventvalue2% press: %eventvalue3%"
 endon

There is a number of reasons to combine all task values in a single event:

* Less events to process, as the rules have to be parsed for each event.
* All task values of the same read are present at the same time.

Especially the last reason, to have all values present when handling an event, is very useful.
When you need to take an action based on 2 values of the same sensor, you must make sure they both belong to the same sample.

A typical example is to compute the dew point, which is a relation between temperature and (relative) humidity.

.. code-block:: none

 on bme#All do
   LogEntry,"Dew point: %c_dew_th%(%eventvalue1%,%eventvalue2%)"
 endon




Internal variables
------------------

A really great feature to use is the internal variables. You set them like this:

.. code-block:: none

 Let,<n>,<value>

Where n must be a positive integer (type ``uint32_t``) and the value a floating point value. To use the values in strings you can
either use the ``%v7%`` syntax or ``[var#7]``. BUT for formulas you need to use the square
brackets in order for it to compute, i.e. ``[var#12]``.

.. note: The number for ``n`` used to be limited to 1 ... 16, but this limit has been removed in builds made after 2021-01-09.

If you need to make sure the stored value is an integer value, use the ``[int#n]`` syntax. (i.e. ``[int#12]``)
The index ``n`` is shared among ``[var#n]`` and ``[int#n]``.

On the "System Variables" page of the web interface all set values can be inspected including their values.
If none is set, "No variables set" will be shown.

If a specific system variable was never set (using the ``Let`` command), its value will be considered to be ``0.0``.

.. note: Interval variables are lost after a reboot. If you need to keep values that will survive a reboot or crash (without loosing power), please use a dummy task for this.



Special task names
------------------

You must not use the task names ``Plugin``, ``var`` ``int`` as these have special meaning.

``Plugin`` can be used in a so called ``PLUGIN_REQUEST``, for example: 
``[Plugin#GPIO#Pinstate#N]`` to get the pin state of a GPIO pin.

``[Plugin#MCP#Pinstate#N]`` to get the pin state of a MCP pin.

``[Plugin#PCF#Pinstate#N]`` to get the pin state of a PCF pin.

For expanders you can use also the following:

``[Plugin#MCP#PinRange#x-y]`` to get the pin state of a range of MCP pins from x o y.

``[Plugin#PCF#PinRange#x-y]`` to get the pin state of a range of PCF pins from x o y.

``Var`` and ``int`` are used for internal variables. 
The variables set with the ``Let`` command will be available in rules
as ``var#N`` or ``int#N`` where ``N`` is 1..16.
For example: ``Let,10,[var#9]``

N.B. ``int`` and ``var`` use the same variable, only ``int`` does round them to 0 decimals.
N.B.2  ``int`` is added in build 20190916.

``Clock``, ``Rules`` and ``System`` etc. are not recommended either since they are used in
event names.

Please observe that task names are case insensitive meaning that VAR, var, and Var etc.
are all treated the same.


Parameter parsing
-----------------

A command used in rules can have several parameters.
A parameter is typically separated by a comma, but for convenience the separator can also 
be accompanied by a space or be a single space.

So these can be used to separate a parameter:

* comma
* comma space
* space comma
* space
* space space

This does impose some restrictions on the use of a comma or a space in a parameter.
Especially sending JSON to a MQTT controller can become next to impossible with these limitations.

In order to allow the comma or space in a parameter, you can wrap the parameter in quotes.

* Single quote (')
* Double quote (")
* Back quote  (added to builds after 2019/11/10)

There are multiple quotes available for this, to be able to use "the other quote" in your parameter.
For example in JSON, you need the double quote for string like values or keys.

.. code-block:: none
 
 Publish domoticz/in,{"idx":26,"nvalue":0,"svalue":"[AQ#TVOC]"}

This can be fixed by wrapping the last parameter with single quotes like this:

.. code-block:: none
 
 Publish domoticz/in,'{"idx":26,"nvalue":0,"svalue":"[AQ#TVOC]"}'

Just make sure to use the same quote as start and end of your parameter and don't use that character in the parameter itself.
N.B. these extra quotes are removed from the parameter when used, as well as trailing or leading spaces.

The reason this behavior was changed from before 2019/11 was that the old implementation could lead to unpredictable results.


Formatting refered values
-------------------------

When referring another value, some basic formatting can be used.

Referring a value using some pre-defined format: ``[TaskName#ValueName#transformation#justification]``

Transformation
^^^^^^^^^^^^^^

* Transformations are case sensitive. (``M`` differs from ``m``, capital is more verbose)
* Transformations can not be used on "Plugin" calls, like ``[Plugin#GPIO#Pinstate#N]``, since these already use multiple occurences of ``#``.
* Most transformations work on "binary" values (logic values 0 or 1)
* A "binary" transformation can be "inverted" by adding a leading ``!``.
* A "binary" value is considered 0 when its string value is "0" or empty, otherwise it is an 1. (float values are rounded)
* A "binary" value can also be used to detect presence of a string, as it is 0 on an empty string or 1 otherwise.
* If the transformation contains ``R``, under certain circumstances, the value will be right-aligned.

Binary transformations:

* ``C``: 0 => "CLOSE" 1 => " OPEN"
* ``c``: 0 => "CLOSED" 1 => "  OPEN"
* ``H``: 0 => "COLD" 1 => " HOT"
* ``I``: 0 => "OUT" 1 => " IN"
* ``L``: 0 => " LEFT" 1 => "RIGHT"
* ``l``: 0 => "L" 1 => "R"
* ``M``: 0 => "AUTO" 1 => " MAN"
* ``m``: 0 => "A" 1 => "M"
* ``O``: 0 => "OFF" 1 => " ON"
* ``U``: 0 => "DOWN" 1 => "  UP"
* ``u``: 0 => "D" 1 => "U"
* ``V``: value = value without transformations
* ``X``: 0 => "O" 1 => "X"
* ``Y``: 0 => " NO" 1 => "YES"
* ``y``: 0 => "N" 1 => "Y"
* ``Z``: 0 => "0" 1 => "1"

Floating point transformations:

* ``Dx.y``: Minimal 'x' digits zero filled & 'y' decimal fixed digits. E.g. ``[bme#T#D2.1]``
* ``Dx``: Minimal 'x' digits zero filled in front of the decimal point, no decimal digits. Same as ``Dx.0``
* ``D.y``: Same as ``D0.y``
* ``d``: Same as ``D`` but spaces insted zeros
* ``F``: Floor (round down)
* ``E``: cEiling (round up)

Other transformations:

* ``p``: Password display, replacing all value characters by asterisks ``*``. If the value is "0", nothing will be displayed.
* ``pc``: Password display with custom character ``c``. For example p- will display value "123" as "---". If the value is "0", nothing will be displayed.

Justification
^^^^^^^^^^^^^

To apply a justification, a transformation must also be used. If no transformation is needed, use the ``V`` (value) transformation.

* ``Pn``: Prefix Fill with n spaces.
* ``Sn``: Suffix Fill with n spaces.
* ``Ln``: Left part of the string, n characters.
* ``Rn``: Right part of the string, n characters.
* ``Ux.y``: Substring Ux.y where x=firstChar and y=number of characters.
* ``C``: Capitalize first character of each word (space/period separated).
* ``u``: Uppercase entire value.
* ``l``: Lowercase entire value.


String Formatting and Interpreting
----------------------------------

(added 2020/02/24)

String operator commands described here can be recognized by their wrapping curly braces.

This helps recognize task values (``[taskname#varname]``) in these commands.


Substring
^^^^^^^^^

It is possible to process sub strings, for example when working with ``%eventvalue%`` in rules.

Usage: ``{substring:<startpos>:<endpos>:<string>}``

The position arguments are the same as in Arduino ``String::substring`` , meaning the endpos is 1 position further than the last character you need.

For example:

.. code-block:: none
 
 on DS-1#Temperature do
   logentry,{substring:0:1:%eventvalue%}
   logentry,{substring:1:2:%eventvalue%}
   logentry,{substring:2:3:%eventvalue%}
 endon

The ``%eventvalue%`` may contain the value "23.06"
The output in the log will then be:

.. code-block:: none

 1512372 : Info  : EVENT: DS-1#Temperature=23.06
 1512404 : Info  : ACT  : logentry,2
 1512405 : Info  : Command: logentry
 1512406 : Info  : 2
 1512409 : Info  : ACT  : logentry,3
 1512410 : Info  : Command: logentry
 1512410 : Info  : 3
 1512413 : Info  : ACT  : logentry,.
 1512414 : Info  : Command: logentry
 1512415 : Info  : .


N.B. it is also possible to concatenate these and refer to ``{taskname#varname}``.

For example (bit useless example, just for illustrative purposes): 

.. code-block:: none

 on DS-1#Temperature do
   logentry,{substring:0:2:{strtol:16:{substring:0:2:[DS-1#Temperature]}{substring:3:5:[DS-1#Temperature]}}}
 endon

.. code-block:: none

 221313 : Info  : EVENT: DS-1#Temperature=22.13
 221346 : Info  : parse_string_commands cmd: substring:0:2:22.13 -> 22
 221347 : Info  : parse_string_commands cmd: substring:3:5:22.13 -> 13
 221348 : Info  : parse_string_commands cmd: strtol:16:2213 -> 8723
 221349 : Info  : parse_string_commands cmd: substring:0:2:8723 -> 87
 221350 : Info  : ACT  : logentry,87
 221351 : Info  : Command: logentry
 221353 : Info  : 87

strtol
^^^^^^

Strings or substrings can be converted from just about any base value (binary, octal, hexadecimal) into an integer value.

Usage:

* ``{strtol:16:<string>}``  to convert HEX (base 16) into an integer value.
* ``{strtol:2:<string>}``  to convert BIN (base 2) into an integer value.

Example of extracting sub strings from a value and interpreting as if they were HEX values:

.. code-block:: none

 on DS-1#Temperature do
   logentry,{strtol:16:%eventvalue%}
   logentry,{strtol:16:{substring:3:5:%eventvalue%}}
 endon

.. code-block:: none

 1987550 : Info  : EVENT: DS-1#Temperature=24.12
 1987586 : Info  : ACT  : logentry,36
 1987587 : Info  : Command: logentry
 1987588 : Info  : 36
 1987591 : Info  : ACT  : logentry,18
 1987592 : Info  : Command: logentry
 1987593 : Info  : 18

What we see here is the interpretation of "24.12":

* 0x24 = 36
* 0x12 = 18

Example use case:


As a use case, imagine the output of ser2net (P020) from an OpenTherm gateway.

* Message coming from the serial interface: **T101813C0**
  * The B denotes that the message is from the
  * The next 4 bytes (actually 2bytes hex encoded) denote the status and type of the message.
  * the last 4 bytes (actually 2bytes hex encoded) denote the payload.
* Message that ends up in rules when using ser2net (P020) and Generic handling: ``!Serial#BT101813C0``

The room temperature in this sample is 19.75 C

Get the last four bytes in packs of two bytes:

* ``{substring:13:15:%eventvalue%}``
* ``{substring:15:17:%eventvalue%}``

Parsing them to decimal representation each (using a base 16 call to strtol):

* ``{strtol:16:{substring:13:15:%eventvalue%}}``
* ``{strtol:16:{substring:15:17:%eventvalue%}}``

Last but not least the fraction is not correct, it needs to be divided by 256 (and multiplied by 100)

* ``{strtol:16:{substring:15:17:%eventvalue%}}*100/255``

Complete rule used to parse this and set a variable in a dummy device:

.. code-block:: none

 // Room temperature
 on !Serial#T1018* do
   TaskValueSet 2,1,{strtol:16:{substring:13:15:%eventvalue%}}.{strtol:16:{substring:15:17:%eventvalue%}}*100/255
 endon


toBin / toHex
^^^^^^^^^^^^^

(Added: 2020-12-28)

Convert an integer value into a binary or hexadecimal representation.

Usage: 

* ``{toBin:<value>}`` Convert the number into binary representation.
* ``{toHex:<value>}`` Convert the number into hexadecimal representation.

* ``<value>`` The number to convert, if it is representing a valid unsigned integer value.


For example:

.. code-block:: none

 on myevent do
   let,1,%eventvalue1%
   let,2,{bitset:9:%eventvalue1%}
   LogEntry,'Values {tobin:[int#1]} {tohex:[int#1]}'
   LogEntry,'Values {tobin:[int#2]} {tohex:[int#2]}'
 endon


.. code-block:: none

 320528: HTTP: Event,eventname=123
 320586: EVENT: eventname=123
 320594: ACT : let,1,123
 320603: ACT : let,2,635
 320612: ACT : LogEntry,'Values 1111011 7b'
 320618: Values 1111011 7b
 320631: ACT : LogEntry,'Values 1001111011 27b'
 320635: Values 1001111011 27b

ord
^^^

Give the ordinal/integer value of the first character of a string.  (e.g. ASCII integer value)

Usage: ``{ord:<string>}``

For example:

.. code-block:: none

 on DS-1#Temperature do
   logentry,{ord:A}   // ASCII value of 'A'
   logentry,{ord:{substring:2:3:%eventvalue%}}  // ASCII value of 3rd character of %eventvalue%
 endon


.. code-block:: none

 2982455 : Info  : EVENT: DS-1#Temperature=23.12
 2982487 : Info  : ACT  : logentry,65
 2982488 : Info  : Command: logentry
 2982489 : Info  : 65
 2982492 : Info  : ACT  : logentry,46
 2982493 : Info  : Command: logentry
 2982494 : Info  : 46


bitread
^^^^^^^

(Added: 2020-12-28)

Read a specific bit of a number.

Usage: ``{bitRead:<bitpos>:<string>}``

* ``<bitpos>`` Which bit to read, starting at 0 for the least-significant (rightmost) bit.
* ``<string>`` The number from which to read, if it is representing a valid unsigned integer value.

.. note::

  Bitwise operators act on ``unsigned integer`` types, thus negative numbers will be ignored.

.. note::

  The order of parameters differs from the "Arduino" command ``bitRead()``

For example:

.. code-block:: none

 on myevent do
   logentry,{bitread:0:123}   // Get least significant bit of the given nr '123' => '1'
   logentry,{bitread:%eventvalue1%:%eventvalue1%}  // Get bit nr given by 1st eventvalue from 2nd eventvalue => Either '0' or '1'
 endon

bitset / bitclear
^^^^^^^^^^^^^^^^^

(Added: 2020-12-28)

To set or clear a specific bit of a number to resp. '1' or '0'.

Usage:

* ``{bitSet:<bitpos>:<string>}``  Set a specific bit of a number to '1'.
* ``{bitClear:<bitpos>:<string>}``  Set a specific bit of a number to '0'.

With:

* ``<bitpos>`` Which bit to set, starting at 0 for the least-significant (rightmost) bit.
* ``<string>`` The number from which to read, if it is representing a valid unsigned integer value.

.. note::

  Bitwise operators act on ``unsigned integer`` types, thus negative numbers will be ignored.

.. note::

  The order of parameters differs from the "Arduino" commands ``bitSet()`` and ``bitClear()``

For example:

.. code-block:: none

 on myevent do
   logentry,{bitset:0:122}     // Set least significant bit of the given nr '122' to '1' => '123'
   logentry,{bitclear:0:123}   // Set least significant bit of the given nr '123' to '0' => '122'
   logentry,{bitset:%eventvalue1%:%eventvalue1%}  // Set bit nr given by 1st eventvalue to '1' from 2nd eventvalue
 endon

bitwrite
^^^^^^^^

(Added: 2020-12-28)

To set a specific bit of a number to a given value.

Usage: ``{bitWrite:<bitpos>:<string>:<bitval>}``

* ``<bitpos>`` Which bit to set, starting at 0 for the least-significant (rightmost) bit.
* ``<string>`` The number from which to read, if it is representing a valid unsigned integer value.
* ``<bitval>`` The value to set in the given number. N.B. only the last bit of this integer parameter is used. (Thus '0' and '2' as parameter will give the same result)

.. note::

  Bitwise operators act on ``unsigned integer`` types, thus negative numbers will be ignored.

.. note::

  The order of parameters differs from the "Arduino" command bitSet()

For example:

.. code-block:: none

 on myevent do
   logentry,{bitwrite:0:122:1}   // Set least significant bit of the given nr '122' to '1' => '123'
 endon

XOR / AND / OR
^^^^^^^^^^^^^^

(Added: 2020-12-28)

Perform bitwise logic operations XOR/AND/OR

.. note::

  Bitwise operators act on ``unsigned integer`` types, thus negative numbers will be ignored.


Usage: 

* ``{XOR:<uintA>:<uintB>}``
* ``{AND:<uintA>:<uintB>}``
* ``{OR:<uintA>:<uintB>}``

With:

* ``<uintA>`` The first number, if it is representing a valid unsigned integer value.
* ``<uintB>`` The second number, if it is representing a valid unsigned integer value.

For example:

* ``{xor:127:15}`` to XOR the binary values ``1111111`` and ``1111`` => ``1110000``
* ``{and:254:15}`` to AND the binary values ``11111110`` and ``1111`` => ``1110``
* ``{or:254:15}`` to OR the binary values ``11111110`` and ``1111`` => ``11111111``

.. code-block:: none

 on eventname do
   let,1,%eventvalue1%
   let,2,{abs:%eventvalue2%}
   let,3,{and:[int#1]:[int#2]}
   LogEntry,'Values {tobin:[int#1]} AND {tobin:[int#2]} -> {tobin:[int#3]}'
 endon

 
 1021591: EVENT: eventname=127,15
 1021601: ACT : let,1,127
 1021611: ACT : let,2,15.00
 1021622: ACT : let,3,15
 1021639: ACT : LogEntry,'Values 1111111 AND 1111 -> 1111'
 1021643: Values 1111111 AND 1111 -> 1111


Abs
^^^

(Added: 2020-12-28)

Perform ABS on integer values.

Usage:  ``abs(<value>)``

With:

* ``<value>`` The number to convert into an absolute value, if it is representing a valid numerical value.

For example:

* ``abs(-1)`` Return the absolute value => 1

.. note::

  Bitwise operators act on ``unsigned integer`` types, thus negative numbers will be ignored.
  This makes the use of ''abs'' necessary for using bitwise operators if the value may become negative. 

.. code-block:: none

 on eventname do
   let,1,%eventvalue1%                   // Don't change the value
   let,2,{bitset:9:abs(%eventvalue1%)}   // Convert to positive and set bit '9'
   LogEntry,'Values {tobin:[int#1]} {tohex:[int#1]}'
   LogEntry,'Values {tobin:[int#2]} {tohex:[int#2]}'
 endon

Called with ``Event,eventname=-123`` :

.. code-block:: none  

 110443: EVENT: eventname=-123
 110452: ACT : let,1,-123
 110462: ACT : let,2,635
 110475: ACT : LogEntry,'Values {tobin:-123} {tohex:-123}'
 110484: Values {tobin:-123} {tohex:-123}
 110496: ACT : LogEntry,'Values 1001111011 27b'
 110500: Values 1001111011 27b

As can be seen in the logs, when calling bitwise operators with negative numbers, the value is ignored and thus the expression is still visible in the output.
Therefore make sure to use the ``abs`` function before handing the value over to binary logical operators.

Constrain
^^^^^^^^^

(Added: 2020-12-28)

Constrains a number to be within a range.

Usage:  ``{constrain:<value>:<low>:<high>}``

With:

* ``<value>`` The number to constrain, if it is representing a valid numerical value.
* ``<low>`` Lower end of range, if it is representing a valid numerical value.
* ``<high>`` Higher end of range, if it is representing a valid numerical value.

Math Functions
--------------

(Added: 2021-01-10)

ESPEasy also supports some math functions, like trigonometric functions, but also some more basic functions.

Basic Math Functions
^^^^^^^^^^^^^^^^^^^^

* ``log(x)`` Logarithm of x to base 10.
* ``ln(x)`` Natural logarithm of x.
* ``abs(x)`` Absolute value of x.
* ``exp(x)`` Exponential value, e^x.
* ``sqrt(x)`` Square root of x. (x^0.5)
* ``sq(x)`` Square of x, x^2.
* ``round(x)`` Rounds to the nearest integer, but rounds halfway cases away from zero (instead of to the nearest even integer). 

Rules example:

.. code-block:: none  

 on eventname2 do
   let,1,sq(%eventvalue1%)
   let,2,sqrt([var#1])
   let,3,=log(%eventvalue2%)
   let,4,ln(%eventvalue2%)
   LogEntry,'sqrt of [var#1] = [var#2]'
   LogEntry,'log of %eventvalue2% = [var#3]'
   LogEntry,'ln of %eventvalue2% = [var#4]'   
 endon

Called with event ``eventname2=1.234,100``

.. code-block:: none  

 213293 : Info   : EVENT: eventname2=1.234,100
 213307 : Info   : ACT  : let,1,sq(1.234)
 213316 : Info   : ACT  : let,2,sqrt(1.522756)
 213328 : Info   : ACT  : let,3,=log(100)
 213337 : Info   : ACT  : let,4,ln(100)
 213346 : Info   : ACT  : LogEntry,'sqrt of 1.522756 = 1.234'
 213351 : Info   : sqrt of 1.522756 = 1.234
 213357 : Info   : ACT  : LogEntry,'log of 100 = 2'
 213361 : Info   : log of 100 = 2
 213369 : Info   : ACT  : LogEntry,'ln of 100 = 4.60517018598809'
 213374 : Info   : ln of 100 = 4.60517018598809



Trigonometric Functions
^^^^^^^^^^^^^^^^^^^^^^^

Since the trigonometric functions add quite a bit to the compiled binary, these functions are not included in builds which have a flag defined to limit their build size.

All trigonometric functions are present in 2 versions, for angles in radian and with the ``_d`` suffix for angles in degree.

Radian Angle:

* ``sin(x)`` Sine of x (radian)
* ``cos(x)`` Cosine of x (radian)
* ``tan(x)`` Tangent of x (radian)
* ``aSin(x)`` Arc Sine of x (radian)
* ``aCos(x)`` Arc Cosine of x (radian)
* ``aTan(x)`` Arc Tangent of x (radian)

Degree Angle:

* ``sin_d(x)`` Sine of x (degree)
* ``cos_d(x)`` Cosine of x (degree)
* ``tan_d(x)`` Tangent of x (degree)
* ``aSin_d(x)`` Arc Sine of x (degree)
* ``aCos_d(x)`` Arc Cosine of x (degree)
* ``aTan_d(x)`` Arc Tangent of x (degree)




System variables
----------------

There is a large number of system variables.
These do not refer to task values, but to typical system variables like system uptime, current time and date, etc.

These can all be seen on the ``<ip-address>/sysvars`` page.

N.B. These values cannot be formatted like the task value references.


Best practice
-------------

It is possible to use CAPITAL letters and lower case as you please but best
practice is to use the same types of letters that are found in the
[ADD_LINK], and plugin specific commands. For the logics (on, if, else ... )
the general idea is to use lower case.

Regarding spaces in names it is recommended to NOT use them as it makes bug
testing rules a lot harder. Spaces between chunks of code is possible to make
the code more readable:

.. code-block:: none

 [DeviceName#ValueName]<<value> //These work...
 [DeviceName#ValueName] < <value> //the same...


Sometimes there is limited space to use a reference, like in some plugins 
or when the maximum size of a rule file has been reached.

In such cases it is adviced to use short names for tasks and values.
For example: ``[bme#T]`` instead of ``[bme280#temperature]`` 

Some working examples
=====================

TaskValueSet
------------

Dummy Device is a single way to store and read value on variable.
Just create Generic - Dummy Device and variables inside it.

.. code-block:: none

 TaskValueSet,TASKnr,VARnr,Value

Alternatively, TASKname and/or VARname can be used instead of TASKnr and VARnr:

 .. code-block:: html

 TaskValueSet,TASKname,VARname,Value
 TaskValueSet,TASKnr,VARname,Value
 TaskValueSet,TASKname,VARnr,Value

This example for two switches that toggle one device (LED and Relay on GPIO 13 and 16).


.. code-block:: none

 on sw1#state do
  if [dummy#var1]=0
    TaskValueSet 12,1,0
  else
    TaskValueSet 12,1,1
  endif
  gpio,16,[dummy#var1]
  gpio,13,[dummy#var1]
 endon

 on sw1a#state do
  if [dummy#var1]=0
    TaskValueSet 12,1,1
  else
    TaskValueSet 12,1,0
  endif
  gpio,16,[dummy#var1]
  gpio,13,[dummy#var1]
 endon

 // Alternative for above example using TASKname/VARname
 on sw1#state do
  if [dummy#var1]=0
    TaskValueSet dummy,var1,0
  else
    TaskValueSet dummy,var1,1
  endif
  gpio,16,[dummy#var1]
  gpio,13,[dummy#var1]
 endon

 on sw1a#state do
  if [dummy#var1]=0
    TaskValueSet dummy,var1,1
  else
    TaskValueSet dummy,var1,0
  endif
  gpio,16,[dummy#var1]
  gpio,13,[dummy#var1]
 endon

Please note that the values stored in a Dummy Value are of type float.
This does mean you only have about 20 bits of resolution for the value.

Storing large numbers like the unix time (31 bits of resolution needed) do need some tricks to be stored.
For the Unix time there are now 2 variables included:

- %unixday%
- %unixday_sec%

Here some example used to store the Unix time in the dummy plugin to keep track of actions.
The values stored in the Dummy variables will be kept and restored on a crash/reboot as long as the ESP remains powered.

.. code-block:: none

 if [DT#YMD]=0 and %unixday%>0
  taskvalueset,7,1,%unixday%-1
 endif
 if %unixday%>0
  let,5,%unixday%-[DT#YMD]
  let,4,%v5%*86400-[DT#HMS]+%unixday_sec%
 else
  let,4,0
 endif
 if %v4%>[Config#MinWateringDelay] 
  event,Irrigate
 endif


Averaging filters
-----------------

You may want to clear peaks in otherwise jumpy measurements and if you cannot
remove the jumpiness with hardware you might want to add a filter in the software.

A **10 value average**:

.. code-block:: none

  On Temp#Value Do
   Let,10,[VAR#9]
   Let,9,[VAR#8]
   Let,8,[VAR#7]
   Let,7,[VAR#6]
   Let,6,[VAR#5]
   Let,5,[VAR#4]
   Let,4,[VAR#3]
   Let,3,[VAR#2]
   Let,2,[VAR#1]
   Let,1,[Temp#Value]
   TaskValueSet,12,1,([VAR#1]+[VAR#2]+[VAR#3]+[VAR#4]+[VAR#5]+[VAR#6]+[VAR#7]+[VAR#8]+[VAR#9]+[VAR#10])/10
  EndOn

In the above example we use the sensor value of ``Temp#Value`` to get the trigger event,
we then add all the previous 9 values to the internal variables and the newly acquired
value to the first variable. We then summarize them and divide them by 10 and store it
as a dummy variable (example is on task 12, value 1) which we use to publish the sliding
value instead of the sensor value.

Another filter could be to just use the previous value and **dilute** the new value with that one:

.. code-block:: none

  On Temp#Value Do
    Let,2,[VAR#1]
    Let,1,[Temp#Value]
    TaskValueSet,12,1,(3*[VAR#1]+[VAR#2])/4
  EndOn


Yet another filter could be to add the new value to a **summarized average**:

.. code-block:: none

  On Temp#Value Do
    Let,1,[Temp#Value]
    TaskValueSet,12,1,([VAR#1]+3*[VAR#2])/4
    Let,2,[Dummy#Value]
  EndOn

What you should use? That is a case by case question. Try them all and see which
one suits your actual scenario the best.

PIR and LDR
-----------

.. code-block:: none 

 On PIR#State do
   if [LDR#Light]<500
     gpio,16,[PIR#State]
   endif
 endon

.. note::

  In other words: If the PIR switch is set (to either 1 or 0) and if
  the light value < 500, then set GPIO port 16 of the ESP.

.. code-block:: none

 on PIR#State=1 do
   if [LDR#Light]<500
     gpio,16,[PIR#State]
   endif
 endon

Now the event is only triggered when the PIR switches on.

SR04 and LDR
------------

.. code-block:: none

 on SR04#range<100 do
   if [ldr#lux]<500
     gpio,2,0
     gpio,16,1
   else
     gpio,2,1
     gpio,16,0
   endif
 endon


Timer
-----

Until 2020/08/12, there were 8 timers.  (1-8)
Builds made after this date support 256 timers.  (1-256)

.. code-block:: none

 On System#Boot do    //When the ESP boots, do
   servo,1,12,0
   timerSet,1,10      //Set Timer 1 for the next event in 10 seconds
 endon

 On Rules#Timer=1 do  //When Timer1 expires, do
   servo,1,12,30
   timerSet,2,1       //Set Timer 2 for the next event in 1 second
 endon

 On Rules#Timer=2 do  //When Timer2 expires, do
   servo,1,12,0
   timerSet,1,30      //Set Timer1 for the next event in 30 seconds
 endon


Timers can also be paused and resumed using resp. ``timerPause`` and ``timerResume``.


Sub-second resolution and loop timers
-------------------------------------

Added on 2020/08/12:

* ``timerSet_ms``  To set the timer with msec resolution.
* ``loopTimerSet`` To create a repeating timer with constant interval.
* ``loopTimerSet_ms`` Same as ``loopTimerSet``, with msec interval.

Here a small example to show how to start/stop and pause loop timers.
This can be used to create quite complex timing schemas, especially when
using multiple timers which are set to a relative prime interval.

N.B. the 2nd eventvalue of ``Rules#Timer`` has the number of loops.

.. code-block:: html

 On System#Boot do    //When the ESP boots, do
   looptimerset_ms,1,2000,10  // Start loop timer 1, 2000 msec interval, 10 loops
   looptimerset_ms,2,2500     // Start loop timer 2, 2500 msec interval
 endon
 
 On Rules#Timer=1 do
   if %eventvalue2% >= 5
     timerSet,1,0     // Stop timer 1
   endif
   //pulse some led on pin 2 shortly
   Pulse,2,0,50
   logentry,%eventvalue2%  // log the loop count
 endon
 
 On Rules#Timer=2 do
   if %eventvalue2% = 2
     loopTimerSet_ms,2,2500  // Restart loop timer 2 (thus clearing loop count)
     timerResume,1
   else
     timerPause,1
   endif
 endon






Starting/stopping repeating timers with events
----------------------------------------------

To disable an existing timer, set it to 0. This is useful to make repeating
timers for things like alarms or warnings:

.. code-block:: none

 //start the warning signal when we receive a start_warning event:
 On start_warning do
   timerSet,1,2
 endon

 //stop the warning signal when we receive a stop_warning event:
 On stop_warning do
   timerSet,1,0
 endon

 //create an actual warning signal, every time timer 1 expires:
 On Rules#Timer=1 do
   //repeat after 2 seconds
   timerSet,1,2
   //pulse some led on pin 4 shortly
   Pulse,4,1,100
   //produce a short 1000hz beep via a piezo element on pin 14
   tone,14,1000,100
 endon

To start or stop the warning signal use http:

.. code-block:: none

 http://<espeasyip>/control?cmd=event,start_warning
 http://<espeasyip>/control?cmd=event,stop_warning

HTTP call
---------

When you enter this first command with the correct IP address in the URL of your browser:

.. code-block:: none

 http://<espeasyip>/control?cmd=event,startwatering
 http://<espeasyip>/control?cmd=event,stopwatering

And have this rule in the addressed ESP:

.. code-block:: none

 On startwatering do
  gpio,12,1 //start watering (open valve)
  timerSet,1,600 //timer 1 set for 10 minutes
 endon

 On stopwatering do
  timerSet,1,0 //timer 1 set to halt, used to stop watering before the timer ends!
  gpio,12,0 //stop watering (close valve)
 endon

 On Rules#Timer=1 do
   gpio,12,0 //stop watering (close valve)
 endOn


Provided that you also have the valve etc., the plants will be happy.

SendTo and Publish
------------------

With SendTo you can add a Rule to your ESP Easy, capable of sending an event to another unit.
This can be useful in cases where you want to take immediate action.
There are two flavors:
- SendTo to send remote unit control commands using the internal peer to peer UDP messaging
- Publish to send remote commands to other ESP using MQTT broker

SendTo:  SendTo <unit>,<command>


Imagine you have two ESP Easy modules, ESP#1 and ESP#2
In the Rules section of ESP#1 you have this:

.. code-block:: none

 on demoEvent do
   sendTo,2,event,startwatering //(to use the previous example.)
 endon

And ESP#2 has the rules according to the previous example (givemesomewater)

If you then enter this with the correct IP address in the URL of your browser:

.. code-block:: none

 http://<ESP#1-ip >/control?cmd=event,demoEvent

Then ESP#1 shall send the event 'startwatering ' to ESP#2.

It is also possible to directly order GPIO changes, like:

.. code-block:: none

 on demoEvent do
   sendTo,2,GPIO,2,1
 endon


Publish

.. code-block:: none

 Publish,<topic>,<value>

To be created.

Time
----

With Rules you can also start or stop actions on a given day and time, or even on every day.

.. code-block:: none

 On Clock#Time=All,18:25 do // every day at 18:25 hours do ...
  gpio,14,0
 endon

Or for a specific day:

.. code-block:: none

 On Clock#Time=Sun,18:25 do  // for Sunday, but All, Sun, Mon, Tue, Wed, Thu, Fri, Sat will do.
  gpio,14,0
 endon

It is also possible to use the system value %systime% in rules conditions
to make things happen during certain hours of the day:

.. code-block:: none

  On Pir#State=1 do
   If %systime% < 07:00:00
    Gpio,16,0
   Endif
   If %systime% > 19:00:00
    Gpio,16,1
   Endif
  Endon

This will set GPIO 16 to 1 when the PIR is triggered, if the time is
before 7 in the morning or after 19:00 in the evening
( useful if you don't have a light sensor).

SendToHTTP
----------

To send a message to another device, like a command to switch on a light to Domoticz

.. code-block:: none

 On System#Boot do    //When the ESP boots, do
   timerSet,1,10      //Set Timer 1 for the next event in 10 seconds
 endon

 On Rules#Timer=1 do  //When Timer1 expires, do
   SendToHTTP 192.168.0.243,8080,/json.htm?type=command&param=switchlight&idx=174&switchcmd=On
 endon

Many users have reported problems with commands being truncated, particularly
when trying to send commands to Domoticz. It seems to be a parsing error.
There is the following workaround:

.. code-block:: none

   SendToHTTP 192.168.0.243,8080,/json.htm?type=param=switchlight&command&idx=174&switchcmd=On


Dew Point for temp/humidity sensors (BME280 for example)
--------------------------------------------------------

If you have a sensor that is monitoring the air temperature and the relative
humidity you may calculate the dew point with rules. This example use MQTT to
publish the values but you may change this to whatever you want. We also make
use of a 'dummy device' to dump values, this example use two BME280 with
different i2c addresses.

For dew point on the 'outside':

.. code-block:: none

 on TempHumidityPressure_OUTSIDE#%RH do
  TaskValueSet,7,1,[TempHumidityPressure_OUTSIDE#°C]-(100-[TempHumidityPressure_OUTSIDE#%RH])/5  // "7" is the number of the task that the dummy device is on, "1" is its first value where we dump our result
  if [TempHumidityPressure_OUTSIDE#%RH]>49
   Publish,%sysname%/DewPoint_OUTSIDE/°C,[Dew_point#°C1]
  else
   Publish,%sysname%/DewPoint_OUTSIDE/°C,[Dew_point#°C1]*  //This asterix shows that the calculation is not correct due to the humidity being below 50%!
  endif
 endon

For dew point on the 'inside':

.. code-block:: none

 on TempHumidityPressure_INSIDE#%RH do
  TaskValueSet,7,2,[TempHumidityPressure_INSIDE#°C]-(100-[TempHumidityPressure_INSIDE#%RH])/5  // "7" is the number of the task that the dummy device is on, "2" is its second value where we dump our result
  if [TempHumidityPressure_INSIDE#%RH]>49
   Publish,%sysname%/DewPoint_INSIDE/°C,[Dew_point#°C2]
  else
   Publish,%sysname%/DewPoint_INSIDE/°C,[Dew_point#°C2]*  //This asterix shows that the calculation is not correct due to the humidity being below 50%!
  endif
 endon


Report IP every 30 seconds using MQTT
-------------------------------------

This rule also work as a ping or heart beat of the unit. If it has not
published a IP number for 30+ seconds the unit is experiencing problems.

.. code-block:: none

 On System#Boot do    //When the ESP boots, do
  Publish,%sysname%/IP,%ip%
  timerSet,1,30      //Set Timer 1 for the next event in 30 seconds
 endon

 On Rules#Timer=1 do  //When Timer1 expires, do
  Publish,%sysname%/IP,%ip%
  timerSet,1,30       //Resets the Timer 1 for another 30 seconds
 endon

Custom reports to Domoticz with own IDX
---------------------------------------

This rule was presented as a workaround for a problem where a sensor had
three different values but only one IDX value. You could publish your own
Domoticz messages (MQTT or HTTP) using this method. Below we use the INA219
plugin that have 3 values which of the two second ones are Amps and Watts,
just as an example we want to publish these as custom messages with a unique IDX value.

*MQTT*

.. code-block:: none

 on INA219#Amps do
  Publish domoticz/in,{"idx":123456,"nvalue":0,"svalue":"[INA219#Amps]"} //Own made up IDX 123456
 endon

 on INA219#Watts do
  Publish domoticz/in,{"idx":654321,"nvalue":0,"svalue":"[INA219#Watts]"} //Own made up IDX 654321
 endon


*HTTP*

.. code-block:: none

 on INA219#Amps do
  SendToHTTP 192.168.1.2,8080,/json.htm?type=command&param=udevice&idx=123456&nvalue=0&svalue=[INA219#Amps] //Own made up IDX 123456
 endon

 on INA219#Watts do
  SendToHTTP 192.168.1.2,8080,/json.htm?type=command&param=udevice&idx=654321&nvalue=0&svalue=[INA219#Watts] //Own made up IDX 654321
 endon

(Given that your Domoticz server is on "192.168.1.2:8080", you should change
to your server IP and PORT number. If the HTTP publishing is not working,
please refer to this [ADD_LINK] for a workaround.)

Authentication to Domoticz via SendToHTTP
-----------------------------------------

It is possible to use authentication in Domoticz and use it via SendToHTTP.

* MkE= is the base64 encoded username ('2A' in this example)
* OVM= is the base64 encoded password ('9S' in this example)

``SendToHTTP xx.xx.xx.xx,8080,/json.htm?username=MkE=&password=OVM&type=command&param=switchlight&idx=36&switchcmd=On``

See also `Domoticz Wiki <https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Authorization>`_

One button, multiple actions using long press
---------------------------------------------

Using a "normal switch" device which is in this example normally set to low
(0) you can make one of two actions  when pressed. If you either release the
button in less than a second or press it for more than a second:

.. code-block:: none

 on Button#State=1 do
  timerSet,1,1
 endon

 on rules#timer=1 do
  if [Button#State]=0
   //Action if button is short pressed
  else
   //Action if button is still pressed
  endif
 endon

Calculating water consumption
-----------------------------

Using the pulse counter you can calculate and act on waterflow and changes like this:

.. code-block:: none

 On System#Boot do // When the ESP boots, do
  TaskValueSet,3,1,0 // TaskValueSet TASKnr,VARnr,Value, Reset the Liters counter to 0
  TaskValueSet,3,2,0 // TaskValueSet TASKnr,VARnr,Value, Reset the PreviousLiters counter to 0
  TaskValueSet,3,3,0 // TaskValueSet TASKnr,VARnr,Value, Reset the Flow counter to 0
  TaskValueSet,3,4,0 // TaskValueSet TASKnr,VARnr,Value, Reset the PreviousFlow counter to 0
  TimerSet,1,30 // Set Timer 1 for the next event in 30 seconds
 EndOn

 On Watermeter#Count do // When Pulse is detected
  if [Watermeter#Count] > 0
    SendToHTTP,192.168.1.50,8084,/json.htm?type=command&param=udevice&idx=337&nvalue=0&svalue=1
    TaskValueSet,3,3,60000/[Watermeter#Time]
    SendToHTTP,192.168.1.50,8084,/json.htm?type=command&param=udevice&idx=338&nvalue=0&svalue=[Liters#Flow]
  endif
 EndOn

 On Rules#Timer=1 do // When Timer 1 expires, do
  if [Liters#Flow] > 0 or [Liters#PreviousFlow] > 0 // Only send value if amount of Liters > 0
    SendToHTTP,192.168.1.50,8084,/json.htm?type=command&param=udevice&idx=338&nvalue=0&svalue=[Liters#Flow]
    TaskValueSet,3,4,[Liters#Flow] // set flow to previous counter
    TaskValueSet,3,3,0
  endif
    TimerSet,1,30 // Set Timer 1 for the next event in 30 seconds
 Endon


Iterate over lookup table
-------------------------

Sometimes you need to perform actions in a sequence.
For example turn on a few LEDs in some specific order.
This means you need to keep track of the current step, but also know what specific pin to turn on or off.

Here an example just showing a number of GPIO pins that could be turned on and off.
For the example, the GPIO pin numbers are just sent to a log, but it is easy to convert them to a GPIO command.

.. code-block:: none

 on init do
   // Set the pin lookup sequence
   let,1,1
   let,2,2
   let,3,3
   let,4,-1
   let,15,0  // Used for keeping the position in the sequence
   asyncevent,loop // Trigger the loop
 endon
 
 on run do
   // Use %eventvalue1% as the index for the variable
   if [int#%eventvalue1%] >= 0
     LogEntry,'Off: [int#%eventvalue1%]'
   endif
   if [int#%eventvalue2%] >= 0
     LogEntry,'On : [int#%eventvalue2%]'
   endif
 endon
 
 on loop do
   if [int#15]<4
     let,14,[int#15]   // Store the previous value
     let,15,[int#15]+1 // Increment
     asyncevent,run=[int#14],[int#15]
     asyncevent,loop
   endif
 endon

This can be started by sending the event ``init`` like this: ``event,init``

N.B. the events ``run`` and ``loop`` are not executed immediately, as that will cause a recursion and thus using a lot of memory to process the rules.
Therefore the ``asyncevent`` is used to append the events to a queue.

This can be made much more dynamic as you may trigger a ``taskrun``, which will send an event when new values are read.
Like this it is possible to automate a complex sequence of steps as not only GPIO pins can be stored, but also task indices.



