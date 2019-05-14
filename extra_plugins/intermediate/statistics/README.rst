Statistics (intermediate plugin)
===================================

Intermediate plugin Statistics is intended to collect operational parameters, store them and provide
way of their export. Data are stored in structures which are based on MIB module IPFIX-MIB (RFC6615), so most
of the elements included in this MIB module are implemented in the internal structures as well.

Currently there is only one submodule that can be used for data export, which is SNMP agent.
This agent uses Net-SNMP library, thus this library is needed to be installed for compilation
of whole plugin. SNMP agent uses snmpd service to receive and dispatch SNMP request, so snmpd is
also needed to be installed. After start the agent registers to snmpd service through AgentX protocol, so there
is no need to configure IP addresses or user accounts for SNMP communication inside of the plugin
as the snmpd service handles this itself.

Net-SNMP library offers cache mechanism, which is also implemented in SNMP agent. Caches are used
for storing information for export. These information are transferred from internal data structure
every time the cache expires and new SNMP request appears. Timeout for cache expiration of each MIB
table can be defined in configuration file. If you wish to get the most actual data from collector,
set the timeout to low values (lowest possible value is 1). If you want to save resources and accesses
to internal structures you can adjust the values to your need. Default value of the timeout is set to 1s,
so if you pull values occasionally you don't have to bother with setting the timeouts as the cache will
be mostly in expired state and it will update itself with the new request. But if you use some SNMP manager
application for pulling the values periodically, it is recommended to adjust the timeouts accordingly.

How to build
------------

By default, the plugin is not distributed with IPFIXcol due to extra dependencies.
To build the plugin, IPFIXcol (and its header files) and the following dependencies must be
installed on your system:

- `Net-SNMP <http://www.net-snmp.org/>`_

You can install necessary Net-SNMP packages using this command:

.. code-block:: sh

    # Ubuntu/Debian distributions
    $ sudo apt-get install libsnmp-base libsnmp-dev snmpd

    # Fedora/RHEL distributions
    $ sudo dnf install net-snmp net-snmp-devel

Finally, compile and install the plugin:

.. code-block:: sh

    $ mkdir build && cd build && cmake ..
    $ make
    # make install

Using the module
-----------------
.. warning::

    First of all, the change to *snmpd.conf* file is needed for functionality of the module.
    The file location is */etc/snmp/snmpd.conf*. You can edit it manually or you can use snmpconf script,
    which will guide you through the configuration. Anyway the file **MUST** contain this line

    .. code-block:: sh

        master agentx

    Without it, the statistics module wouldn't be able to connect to the master agent and collector won't start.
    If you done some changes to the file, restart the snmpd service and check the status (also enable the
    start of the service on boot, so you don't have to start the service manually)


    .. code-block:: sh

        # enable the service after boot of the system
        $ sudo systemctl snmpd.service enable
        # start/restart if the service is already  running
        $ sudo systemctl snmpd.service start
        # check the status
        $ sudo systemctl snmpd.service status

    In the status output you should see that the service is "Active" and there should also be line
    "Turning on AgentX master support". This means that the snmpd service is up and ready.

The OID of the IPFIX-MIB module is

.. code-block:: sh

    1.3.6.1.2.1.193
    #or
    iso.org.dod.internet.mgmt.mib-2.ipfixMIB

To poll values from the Statistics module agent you can use one of the tools distributed with Net-SNMP library,
more specifically the snmpget or snmpwalk tools.

.. note::

    On Fedora/RHEL distributions, the tools needs to be installed from package *libsnmp-utils*.
    On Ubuntu/Debian distributions, the tools needs to be installed from package *snmp*.

Before you start using them, you need to first change the snmpd.conf file again - to allow connections to the agent.
The user settings for snmpd can be also created through the snmpconf script. If you want to test the module or use it
just from device, where the collector runs, adding this line should be enough for basic functionality.

.. code-block:: sh

    rocommunity public 127.0.0.1

If you change the localhost network address to other network or subnet, you can access the module
via SNMP remotely, but it is recommended to use user accounts in SNMPv2c or SNMPv3 in sake of security.

The snmpd.conf file for the most basic functionality (requests accepted only from localhost)
can contain only two lines mentioned before, so it will look like this:

.. code-block:: sh

    master agentx
    rocommunity public 127.0.0.1


If you are planning to run collector without root privileges, you also need to check that snmpd.conf contains
this line

.. code-block:: sh

    agentXPerms 777 777

so SNMP agent in module can connect to the master agent. If the connection fails, whole ipfixcol2 won't start.
But currently it is recommended to run collector with root privileges for best functionality.

After that you'll be finished with editing the snmpd.conf file, restart the snmpd service and check that it started correctly

.. code-block:: sh

    $ sudo systemctl restart snmpd.service
    $ sudo systemctl status snmpd.service

You should see "Active" as a status of the service. Now you can start the collector. After some connections with
exporters will be created, you can check the information from Statistics module with snmpwalk like this (considering
that you used the most basic snmpd.conf file provided above)

.. code-block:: sh

    $ snmpwalk -v 2c -c public localhost 1.3.6.1.2.1.193

and you should get bunch of values as the output. If you want to see the names of the variables, you can download
the IPFIX-MIB.txt (available in RFC6615) and import it to other MIB files, located in */usr/share/snmp/mibs*.

.. note::

    IPFIX-MIB module depends on ENTITY-MIB module, which is usually not included in default MIB modules.
    So download ENTITY-MIB.txt and import it into the same location as IPFIX-MIB module

After the import of the IPFIX-MIB module, you can use the snmpwalk command again, but now with extra parameter

.. code-block:: sh

    $ snmpwalk -v 2c -c public -m +IPFIX-MIB localhost 1.3.6.1.2.1.193

Now you should see the names of the variables.

Example configuration
---------------------
.. code-block:: xml

    <intermediate>
        <name>Collector stats</name>
        <plugin>statistics</plugin>
        <params>
            <sessionActivityTimeout>20</sessionActivityTimeout>
            <outputs>
            <snmp>
                <cacheTimeout table="ipfixTransportSessionTable">10</cacheTimeout>
                <cacheTimeout table="ipfixTemplateTable">5</cacheTimeout>
                <cacheTimeout table="ipfixTemplateDefinitionTable">5</cacheTimeout>
                <cacheTimeout table="ipfixTransportSessionStatsTable">1</cacheTimeout>
                <cacheTimeout table="ipfixTemplateStatsTable">5</cacheTimeout>
            </snmp>
            </outputs>
        </params>
    </intermediate>

Parameters
----------

:``sessionActivityTimeout``:
    One of the information stored about session is activity of the session, which indicates active transmission
    between exporter and collector. This timeout is used to determine, when the session becomes inactive if no
    data are being transmitted. [default:10]

:``outputs``:
    Specifying output submodules, which are used for data export from the plugin. Currently only submodule
    available is SNMP module.

    :``snmp``:
        SNMP output submodule. Just by specifying this tag without any extra parameters enables the submodule
        which is initialized with default values

        :``timeout``:
            Timeout of cache expiration for MIB tables. Name of the table must be specified in ``table`` argument.
            Value can be only positive integer. [default:1]

Notes
-----
If you are using Net-SNMP v5.8 the snmp agent will print this message

.. code-block:: sh

    unknown snmp version 193

This cosmetic issue doesn't affect the functionality of the module.
It comes from the Net-SNMP library, which doesn't have straightforward solution for disabling this.
