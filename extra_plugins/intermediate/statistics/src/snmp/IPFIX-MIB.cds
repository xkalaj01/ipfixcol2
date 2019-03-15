                                                              �"The IPFIX MIB defines managed objects for IP Flow
           Information eXport.  These objects provide information about
           managed nodes supporting the IPFIX protocol,
           for Exporters as well as for Collectors.

           Copyright (c) 2012 IETF Trust and the persons identified as
           authors of the code.  All rights reserved.

           Redistribution and use in source and binary forms, with or
           without modification, is permitted pursuant to, and subject
           to the license terms contained in, the Simplified BSD
           License set forth in Section 4.c of the IETF Trust's
           Legal Provisions Relating to IETF Documents
           (http://trustee.ietf.org/license-info)."p"WG charter:
             http://www.ietf.org/html.charters/ipfix-charter.html

           Mailing Lists:
             General Discussion: ipfix@ietf.org
             To Subscribe: http://www1.ietf.org/mailman/listinfo/ipfix
             Archive:
         http://www1.ietf.org/mail-archive/web/ipfix/current/index.html

           Editor:
             Thomas Dietz
             NEC Europe Ltd.
             NEC Laboratories Europe
             Network Research Division
             Kurfuersten-Anlage 36
             Heidelberg  69115
             Germany
 Phone: +49 6221 4342-128
             Email: Thomas.Dietz@neclab.eu

             Atsushi Kobayashi
             NTT Information Sharing Platform Laboratories
             3-9-11 Midori-cho
             Musashino-shi, Tokyo  180-8585
             Japan
             Phone: +81-422-59-3978
             Email: akoba@nttv6.net

             Benoit Claise
             Cisco Systems, Inc.
             De Kleetlaan 6a b1
             Diegem  1831
             Belgium
             Phone:  +32 2 704 5622
             Email: bclaise@cisco.com

             Gerhard Muenz
             Technische Universitaet Muenchen
             Department of Informatics
             Chair for Network Architectures and Services (I8)
             Boltzmannstr. 3
             Garching  85748
             Germany
             Email: muenz@net.in.tum.de" "201206110000Z" "201004190000Z" 5"Fixed errata from RFC 5815.  Published as RFC 6615." )"Initial version, published as RFC 5815."       -- 11 June 2012
               �"This table lists the currently established Transport
           Sessions between an Exporting Process and a Collecting
           Process."                       5"Defines an entry in the ipfixTransportSessionTable."                      "Locally arbitrary, but unique identifier of an entry in
           the ipfixTransportSessionTable.  The value is expected to
           remain constant from a re-initialization of the entity's
           network management agent to the next re-initialization."                       �"The transport protocol used for receiving or transmitting
           IPFIX Messages.  Protocol numbers are assigned by IANA.  A
           current list of all assignments is available from
           <http://www.iana.org/assignments/protocol-numbers/>." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 10."                    )"The type of address used for the source address,
           as specified in RFC 4001.  The InetAddressType supported
           values are ipv4(1) and ipv6(2).  This object is used with
           protocols (specified in ipfixTransportSessionProtocol) like
           TCP (6) and UDP (17) that have the notion of addresses.
           SCTP (132) should use the ipfixTransportSessionSctpAssocId
           instead.  If SCTP (132) or any other protocol without the
           notion of addresses is used, the object MUST be set to
           unknown(0)."                      ]"The source address of the Exporter of the IPFIX Transport
           Session.  This value is interpreted according to the value of
           ipfixTransportSessionAddressType, as specified in RFC 4001.
           This object is used with protocols (specified in
           ipfixTransportSessionProtocol) like TCP (6) and UDP (17) that
           have the notion of addresses.  SCTP (132) should use the
           ipfixTransportSessionSctpAssocId instead.  If SCTP (132) or
           any other protocol without the notion of addresses is used,
           the object MUST be set to a zero-length string."                      ."The type of address used for the destination address,
           as specified in RFC 4001.  The InetAddressType supported
           values are ipv4(1) and ipv6(2).  This object is used with
           protocols (specified in ipfixTransportSessionProtocol) like
           TCP (6) and UDP (17) that have the notion of addresses.
           SCTP (132) should use the ipfixTransportSessionSctpAssocId
           instead.  If SCTP (132) or any other protocol without the
           notion of addresses is used, the object MUST be set to
           unknown(0)."                      n"The destination address of the Collector of the IPFIX
           Transport Session.  This value is interpreted according to
           the value of ipfixTransportSessionAddressType, as specified
           in RFC 4001.  This object is used with protocols
           (specified in ipfixTransportSessionProtocol) like TCP (6)
           and UDP (17) that have the notion of addresses.  SCTP (132)
           should use the ipfixTransportSessionSctpAssocId instead.
           If SCTP (132) or any other protocol without the notion of
           addresses is used, the object MUST be set to a zero-length
           string."                      y"The transport protocol port number of the Exporter.
           This object is used with protocols (specified in
           ipfixTransportSessionProtocol) like TCP (6)
           and UDP (17) that have the notion of ports.  SCTP (132)
           should copy the value of sctpAssocLocalPort if the
           Transport Session is in collecting mode or
           sctpAssocRemPort if the Transport Session is in
           exporting mode.  The association is referenced
           by the ipfixTransportSessionSctpAssocId.
           If any other protocol without the notion of
           ports is used, the object MUST be set to zero."                      �"The transport protocol port number of the Collector.  The
           default value is 4739 for all currently defined transport
           protocol types.  This object is used with protocols
           (specified in ipfixTransportSessionProtocol) like TCP (6)
           and UDP (17) that have the notion of ports.  SCTP (132)
           should copy the value of sctpAssocRemPort if the
           Transport Session is in collecting mode or
           sctpAssocLocalPort if the Transport Session is in
           exporting mode.  The association is referenced
           by the ipfixTransportSessionSctpAssocId.
           If any other protocol without the notion of
           ports is used, the object MUST be set to zero."                      �"The association ID used for the SCTP session between the
           Exporter and the Collector of the IPFIX Transport Session.
           It is equal to the sctpAssocId entry in the sctpAssocTable
           defined in the SCTP MIB.  This object is only valid if
           ipfixTransportSessionProtocol has the value 132 (SCTP).  In
           all other cases, the value MUST be zero." e"RFC 3873, Stream Control Transmission Protocol (SCTP)
           Management Information Base (MIB)."                    $"The mode of operation of the device for the given Transport
           Session.  This object can have the following values:

           exporting(1)
               This value MUST be used if the Transport Session is
               used for exporting Records to other IPFIX Devices;
               i.e., this device acts as Exporter.

           collecting(2)
               This value MUST be used if the Transport Session is
               used for collecting Records from other IPFIX Devices;
               i.e., this device acts as Collector."                      �"On Exporters, this object contains the time in seconds
           after which IPFIX Templates are resent by the
           Exporter.
           On Collectors, this object contains the lifetime in seconds
           after which a Template becomes invalid when it is not
           received again within this lifetime.

           This object is only valid if ipfixTransportSessionProtocol
           has the value 17 (UDP).  In all other cases, the value MUST
           be zero." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 10.3.6 and 10.3.7."                    �"On Exporters, this object contains the time in seconds
           after which IPFIX Options Templates are resent by the
           Exporter.

           On Collectors, this object contains the lifetime in seconds
           after which an Options Template becomes invalid when it is
           not received again within this lifetime.

           This object is only valid if ipfixTransportSessionProtocol
           has the value 17 (UDP).  In all other cases, the value MUST
           be zero." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 10.3.6 and 10.3.7."                    "On Exporters, this object contains the number of exported
           IPFIX Messages after which IPFIX Templates are resent
           by the Exporter.
           On Collectors, this object contains the lifetime in number
           of exported IPFIX Messages after which a Template becomes
           invalid when it is not received again within this lifetime.

           This object is only valid if ipfixTransportSessionProtocol
           has the value 17 (UDP).  In all other cases, the value MUST
           be zero." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 10.3.6 and 10.3.7."                    )"On Exporters, this object contains the number of exported
           IPFIX Messages after which IPFIX Options Templates are
           resent by the Exporter.

           On Collectors, this object contains the lifetime in number
           of exported IPFIX Messages after which an Options Template
           becomes invalid when it is not received again within this
           lifetime.

           This object is only valid if ipfixTransportSessionProtocol
           has the value 17 (UDP).  In all other cases, the value MUST
           be zero." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 10.3.6 and 10.3.7."                    �"On Exporters, the object contains the version number of the
           IPFIX protocol that the Exporter uses to export its data in
           this Transport Session.

           On Collectors, the object contains the version number of the
           IPFIX protocol it receives for this Transport Session.
           If IPFIX Messages of different IPFIX protocol versions are
           transmitted or received in this Transport Session, this
           object contains the maximum version number." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.1."                    $"The status of a Transport Session.  This object can have the
           following values:

           unknown(0)
               This value MUST be used if the status of the
               Transport Session cannot be detected by the equipment.
               This value should be avoided as far as possible.

           inactive(1)
               This value MUST be used for Transport Sessions that
               are specified in the system but are not currently active.
               The value can be used, for example, for Transport
               Sessions that are backup (secondary) sessions in a
               Transport Session group.

           active(2)
               This value MUST be used for Transport Sessions that are
               currently active and transmitting or receiving data."                      �"This table lists the Templates and Options Templates that
           are transmitted by the Exporting Process or received by the
           Collecting Process.

           The table contains the Templates and Options Templates that
           are received or used for exporting data for a given
           Transport Session group and Observation Domain.

           Withdrawn or invalidated (Options) Templates MUST be removed
           from this table."                       -"Defines an entry in the ipfixTemplateTable."                      2"The ID of the Observation Domain for which this Template
           is defined.  This value is used when sending IPFIX Messages.
           The special value of 0 indicates that the Data Records
           exported with this (Options Template) cannot be applied to a
           single Observation Domain." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.1."                     �"This number indicates the Template ID in the IPFIX
           Message.  Values from 0 to 255 are not allowed for Template
           IDs." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.4.1."                    M"This number indicates the Set ID of the Template.  This
           object allows the Template type to be easily retrieved.

           Currently, there are two values defined.  The value 2 is
           used for Sets containing Template definitions.  The value 3
           is used for Sets containing Options Template definitions." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.3.2."                    �"If the Transport Session is in exporting mode
           (ipfixTransportSessionDeviceMode) the time when this
           (Options) Template was last sent to the Collector(s).
           In the specific case of UDP as transport protocol, this
           time is used to know when a retransmission of the
           (Options) Template is needed.

           If the Transport Session is in collecting mode, this object
           contains the time when this (Options) Template was last
           received from the Exporter.  In the specific case of UDP as
           transport protocol, this time is used to know when this
           (Options) Template times out and thus is no longer valid."                      �"On Exporters, this table lists the (Options) Template fields
           of which a (Options) Template is defined.  It defines the
           (Options) Template given in the ipfixTemplateId specified in
           the ipfixTemplateTable.

           On Collectors, this table lists the (Options) Template fields
           of which a (Options) Template is defined.  It defines the
           (Options) Template given in the ipfixTemplateId specified in
           the ipfixTemplateTable."                       7"Defines an entry in the ipfixTemplateDefinitionTable."                      !"The ipfixTemplateDefinitionIndex specifies the order in
           which the Information Elements are used in the (Options)
           Template Record.

           Since a Template Record can contain a maximum of 65535
           Information Elements, the index is limited to this value." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 3.4.1 and 3.4.2."                    z"This indicates the Information Element ID at position
           ipfixTemplateDefinitionIndex in the (Options) Template
           ipfixTemplateId.  This implicitly specifies the data type
           of the Information Element.  The elements are registered
           at IANA.  A current list of assignments can be found at
           <http://www.iana.org/assignments/ipfix/>." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.2.

           RFC 5102, Information Model for IP Flow Information Export."                     �"This indicates the length of the Information Element ID at
           position ipfixTemplateDefinitionIndex in the (Options)
           Template ipfixTemplateId." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.2.

           RFC 5102, Information Model for IP Flow Information Export."                    "IANA enterprise number of the authority defining the
           Information Element identifier in this Template Record.
           Enterprise numbers are assigned by IANA.  A current list of
           all assignments is available from
           <http://www.iana.org/assignments/enterprise-numbers/>.

           This object must be zero(0) for all standard Information
           Elements registered with IANA.  A current list of these
           elements is available from
           <http://www.iana.org/assignments/ipfix/>." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.2.

           RFC 5102, Information Model for IP Flow Information Export."                     "This bitmask indicates special attributes for the
           Information Element:

           scope(0)
               This Information Element is used for scope.

           flowKey(1)
               This Information Element is a Flow Key.

           Thus, we get the following values for an Information Element:

           If neither bit scope(0) nor bit flowKey(1) is set
               The Information Element is neither used for scoping nor
               as Flow Key.
           If only bit scope(0) is set
               The Information Element is used for scoping.
           If only bit flowKey(1) is set
               The Information Element is used as Flow Key.

           Both bit scope(0) and flowKey(1) MUST NOT be set at the same
           time.  This combination is not allowed." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Sections 2 and 3.4.2.1.

           RFC 5102, Information Model for IP Flow Information Export."                    Q"This table lists all exports of an IPFIX Device.

           On Exporters, this table contains all exports grouped by
           Transport Session, Observation Domain ID, Template ID, and
           Metering Process represented by the
           ipfixMeteringProcessCacheId.  Thanks to the ipfixExportIndex,
           the exports can group one or more Transport Sessions to
           achieve a special functionality like failover management,
           load-balancing, etc.  The entries with the same
           ipfixExportIndex, ipfixObservationDomainId,
           and ipfixMeteringProcessCacheId define a Transport
           Session group.  If the Exporter does not use Transport
           Session grouping, then each ipfixExportIndex contains a
           single ipfixMeteringProcessCacheId, and thus a single
           Transport Session; this session MUST have a member type
           value of primary(1).  Transport Sessions referenced in this
           table MUST have a ipfixTransportSessionDeviceMode value of
           exporting(1).

           On Collectors, this table is not needed."                       +"Defines an entry in the ipfixExportTable."                      $"Locally arbitrary, but unique identifier of an entry in
           the ipfixExportTable.  The value is expected
           to remain constant from a re-initialization of the entity's
           network management agent to the next re-initialization.

           A common ipfixExportIndex between two entries from this
           table indicates that there is a relationship between the
           Transport Sessions in ipfixTransportSessionIndex.  The type
           of relationship is expressed by the value of
           ipfixExportMemberType."                      C"The type of member Transport Session in a Transport
           Session group (identified by the value of ipfixExportIndex,
           ipfixObservationDomainId, and ipfixMeteringProcessCacheId).
           The following values are valid:

           unknown(0)
               This value MUST be used if the status of the group
               membership cannot be detected by the equipment.  This
               value should be avoided as far as possible.

           primary(1)
               This value is used for a group member that is used as
               the primary target of an Exporter.  Other group members
               (with the same ipfixExportIndex and
               ipfixMeteringProcessCacheId) MUST NOT have the value
               primary(1) but MUST have the value secondary(2).
               This value MUST also be specified if the Exporter does
               not support Transport Session grouping.  In this case,
               the group contains only one Transport Session.

           secondary(2)
               This value is used for a group member that is used as a
               secondary target of an Exporter.  The Exporter will use
               one of the targets specified as secondary(2) within the
               same Transport Session group when the primary target is
               not reachable.

           parallel(3)
               This value is used for a group member that is used for
               duplicate exporting (i.e., all group members identified
               by the ipfixExportIndex are exporting the same Records
               in parallel).  This implies that all group members MUST
               have the same member type (i.e., parallel(3)).

           loadBalancing(4)
               This value is used for a group member that is used
               as one target for load-balancing.  This means that a
               Record is sent to one of the group members in this
               group identified by ipfixExportIndex.
               This implies that all group members MUST have the same
               member type (i.e., loadBalancing(4))."                      y"This table lists so-called caches used at the Metering
           Process to store the metering data of Flows observed at
           the Observation Points given in the
           ipfixObservationPointGroupReference.  The table lists the
           timeouts that specify when the cached metering data is
           expired.

           On Collectors, the table is not needed."                       4"Defines an entry in the ipfixMeteringProcessTable."                      "Locally arbitrary, but unique identifier of an entry in the
           ipfixMeteringProcessTable.  The value is expected to remain
           constant from a re-initialization of the entity's network
           management agent to the next re-initialization."                      g"The Observation Point Group ID that links this table entry
           to the ipfixObservationPointTable.  The matching
           ipfixObservationPointGroupId in that table gives the
           Observation Points used in that cache.  If the Observation
           Points are unknown, the
           ipfixMeteringProcessObservationPointGroupRef MUST be zero."                      5"On the Exporter, this object contains the time after which a
           Flow is expired (and a Data Record for the Template is sent),
           even though packets matching this Flow are still received by
           the Metering Process.  If this value is 0, the Flow is not
           prematurely expired." Z"RFC 5470, Architecture for IP Flow Information Export,
           Section 5.1.1, item 3."                    �"On the Exporter, this object contains the time after which a
           Flow is expired (and a Data Record for the Template is sent)
           when no packets matching this Flow are received by the
           Metering Process for the given number of seconds.  If this
           value is zero, the Flow is expired immediately; i.e., a Data
           Record is sent for every packet received by the Metering
           Process." Y"RFC 5470, Architecture for IP Flow Information Export,
           Section 5.1.1, item 1"                    #"This table lists the Observation Points used within an
           Exporter by the Metering Process.  The index
           ipfixObservationPointGroupId groups Observation Points
           and is referenced in the Metering Process table.

           On Collectors, this table is not needed."                       5"Defines an entry in the ipfixObservationPointTable."                      �"Locally arbitrary, but unique identifier of an entry in the
           ipfixObservationPointTable.  The value is expected to remain
           constant from a re-initialization of the entity's network
           management agent to the next re-initialization.

           This index represents a group of Observation Points.

           The special value of 0 MUST NOT be used within this table
           but is reserved for usage in the ipfixMeteringProcessTable.
           An index of 0 for the ipfixObservationPointGroupReference
           index in that table indicates that an Observation Point is
           unknown or unspecified for a Metering Process cache."                      l"Locally arbitrary, but unique identifier of an entry in the
           ipfixObservationPointTable.  The value is expected to remain
           constant from a re-initialization of the entity's network
           management agent to the next re-initialization.

           This index represents a single Observation Point in an
           Observation Point group."                       �"The ID of the Observation Domain in which this
           Observation Point is included.

           The special value of 0 indicates that the Observation
           Points within this group cannot be applied to a single
           Observation Domain." �"RFC 5101, Specification of the IP Flow Information Export
           (IPFIX) Protocol for the Exchange of IP Traffic Flow
           Information, Section 3.1."                     �"This object contains the index of a physical entity in
           the ENTITY MIB.  This physical entity is the given
           Observation Point.  If such a physical entity cannot be

           specified or is not known, then the object is zero."                      �"This object contains the index of a physical interface in
           the Interfaces MIB.  This physical interface is the given
           Observation Point.  If such a physical interface cannot be
           specified or is not known, then the object is zero.

           This object MAY be used alone or in addition to
           ipfixObservationPointPhysicalEntity.  If
           ipfixObservationPointPhysicalEntity is not zero, this
           object MUST point to the same physical interface that is
           referenced in ipfixObservationPointPhysicalEntity.
           Otherwise, it may reference any interface in the
           Interfaces MIB."                      �"The direction of the Flow that is monitored on the given
           physical entity.  The following values are valid:

           unknown(0)
               This value MUST be used if a direction is not known for
               the given physical entity.

           ingress(1)
               This value is used for monitoring incoming Flows on the
               given physical entity.

           egress(2)
               This value is used for monitoring outgoing Flows on the
               given physical entity.

           both(3)
               This value is used for monitoring incoming and outgoing
               Flows on the given physical entity."                      j"This table contains Selector Functions connected to a
           Metering Process by the index ipfixMeteringProcessCacheId.
           The Selector Functions are grouped into Selection Processes
           by the ipfixSelectionProcessIndex.  The Selector Functions
           are applied within the Selection Process to the packets
           observed for the given Metering Process cache in increasing
           order as indicated by the ipfixSelectionProcessSelectorIndex.
           This means Selector Functions with a lower
           ipfixSelectionProcessSelectorIndex are applied first.
           The remaining packets are accounted for in Flow Records.

           Since IPFIX does not define any Selector Function (except
           selecting every packet), this is a placeholder for future
           use and a guideline for implementing enterprise-specific
           Selector Function objects.

           The following object tree should help the reader visualize
           how the Selector Function objects should be implemented:

           ipfixSelectorFunctions
           |
           +- ipfixFuncSelectAll
           |  |
           |  +- ipfixFuncSelectAllAvail (is the function available?)
           |
           +- ipfixFuncF2
           |  |
           |  +- ipfixFuncF2Avail (is the function F2 available?)
           |  |
           |  +- ipfixFuncF2Parameters (a table with parameters)
           ...
           |
           +- ipfixFuncFn...
           If a Selector Function takes parameters, the MIB should
           contain a table with an entry for each set of parameters
           used at the Exporter."                       5"Defines an entry in the ipfixSelectionProcessTable."                      "Locally arbitrary, but unique identifier of an entry in the
           ipfixSelectionProcessTable.  The value is expected to remain
           constant from a re-initialization of the entity's network
           management agent to the next re-initialization."                      �"Index specifying the order in which the referenced
           ipfixSelectionProcessSelectorFunctions are applied to the
           observed packet stream within the given Selection Process
           (identified by the ipfixSelectionProcessIndex).  The
           Selector Functions are applied in increasing order; i.e.,
           Selector Functions with a lower index are applied first."                      �"The pointer to the Selector Function used at position
           ipfixSelectionProcessSelectorIndex in the list of Selector
           Functions for the Metering Process cache specified by the
           index ipfixMeteringProcessCacheId and for the given
           Selection Process (identified by the
           ipfixSelectionProcessIndex).

           This usually points to an object in the IPFIX SELECTOR MIB.
           If the Selector Function does not take parameters, then it
           MUST point to the root of the function subtree.  If the
           function takes parameters, then it MUST point to an entry
           in the parameter table of the Selector Function."                           p"This table lists Transport Session statistics between
           Exporting Processes and Collecting Processes."                       :"Defines an entry in the ipfixTransportSessionStatsTable."                       �"The number of bytes per second received by the
           Collector or transmitted by the Exporter.  A
           value of zero (0) means that no packets were sent or
           received yet.  This object is updated every second."                      E"The number of packets received by the Collector
           or transmitted by the Exporter.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      C"The number of bytes received by the Collector
           or transmitted by the Exporter.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      L"The number of IPFIX Messages received by the
           Collector or transmitted by the Exporter.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      _"The number of received IPFIX Messages that are malformed,
           cannot be decoded, are received in the wrong order, or are
           missing according to the sequence number.

           If used at the Exporter, the number of messages that could
           not be sent due to, for example, internal buffer overflows,
           network congestion, or routing issues.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      J"The number of Data Records received by the Collector or
           transmitted by the Exporter.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      "The number of Templates received or transmitted.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      #"The number of Options Templates received or transmitted.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTransportSessionDiscontinuityTime."                      B"The value of sysUpTime at the most recent occasion at which
           one or more of the Transport Session counters suffered a
           discontinuity.
           A value of zero indicates that no such discontinuity has
           occurred since the last re-initialization of the local
           management subsystem."                       3"This table lists statistics objects per Template."                       2"Defines an entry in the ipfixTemplateStatsTable."                      7"The number of Data Records that are transmitted or received
           per Template.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixTemplateDiscontinuityTime."                      "The value of sysUpTime at the most recent occasion at which
           the Template counter suffered a discontinuity.
           A value of zero indicates that no such discontinuity has
           occurred since the last re-initialization of the local
           management subsystem."                       �"This table lists statistics objects that have data per
           Metering Process cache.

           On Collectors, this table is not needed."                       9"Defines an entry in the ipfixMeteringProcessStatsTable."                       5"The number of Flows currently active at this cache."                       %"The number of unused cache entries."                      "The number of Data Records generated.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixMeteringProcessCacheDiscontinuityTime."                      &"The value of sysUpTime at the most recent occasion at which
           the Metering Process counter suffered a discontinuity.
           A value of zero indicates that no such discontinuity has
           occurred since the last re-initialization of the local
           management subsystem."                       �"This table contains statistics for the Selector Functions
           connected to a Metering Process by the index
           ipfixMeteringProcessCacheId.

           The indexes MUST match an entry in the
           ipfixSelectionProcessTable."                       :"Defines an entry in the ipfixSelectionProcessStatsTable."                      �"The number of packets observed at the entry point of the
           function.  The entry point may be the Observation Point or
           the exit point of another Selector Function.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixSelectionProcessStatsDiscontinuityTime."                      &"The number of packets dropped while selecting packets.
           Discontinuities in the value of this counter can occur at
           re-initialization of the management system and at other
           times as indicated by the value of
           ipfixSelectionProcessStatsDiscontinuityTime."                      9"The value of sysUpTime at the most recent occasion at which
           one or more of the Selector counters suffered a
           discontinuity.
           A value of zero indicates that no such discontinuity has
           occurred since the last re-initialization of the local
           management subsystem."                              �"An implementation that builds an IPFIX Collector
           that complies with this module MUST implement the objects
           defined in the mandatory group ipfixCommonGroup.
           The implementation of all objects in the other groups is
           optional and depends on the corresponding functionality
           implemented in the equipment.

           An implementation that is compliant with this MIB module
           is limited to using only the values TCP (6), UDP (17), and
           SCTP (132) in the ipfixTransportSessionProtocol object
           because these are the only protocols currently specified
           for usage within IPFIX (see RFC 5101)."   l"These objects should be implemented if the statistics
           function is implemented in the equipment."            D"An implementation that builds an IPFIX Exporter that
           complies with this module MUST implement the objects defined
           in the mandatory group ipfixCommonGroup.  The implementation
           of all other objects depends on the implementation of the
           corresponding functionality in the equipment."   l"These objects should be implemented if the statistics
           function is implemented in the equipment." h"These objects MUST be implemented if statistics functions
           are implemented in the equipment."                 "The main IPFIX objects."                 "Common statistical objects."                 !"The main objects for Exporters."                 ("The statistical objects for Exporters."                    