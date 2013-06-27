/**************************************************************************
 **
 ** sngrep - SIP callflow viewer using ngrep
 **
 ** Copyright (C) 2013 Ivan Alonso (Kaian)
 ** Copyright (C) 2013 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file pcap.c
 * @author Ivan Alonso [aka Kaian] <kaian@irontec.com>
 *
 * @brief Source of functions defined in pcap.h
 *
 * sngrep can parse a pcap file to display call flows.
 * This file include the functions that uses libpcap to do so.
 *
 * @todo We could request libpcap to filter the file before being processed
 * and only read sip packages. We also allow UDP packages here, and SIP can
 * use other transports, uh.
 *
 */
#include "pcap.h"
#include "sip.h"

int
load_pcap_file(const char* file)
{

    // The header that pcap gives us
    struct pcap_pkthdr header;
    // The actual packet
    const u_char *packet;
    // PCAP file handler
    pcap_t *handle;
    // Error text (in case of file open error)
    char errbuf[PCAP_ERRBUF_SIZE];
    // Packages Datalink
    int linktype;
    // Datalink Header size
    int size_link;
    // Ethernet header data
    struct ether_header *eptr;
    // Ethernet header type
    u_short ether_type;
    // IP header data
    struct nread_ip *ip;
    // IP header size
    int size_ip;
    // UDP header data
    struct nread_udp *udp;
    // XXX Fake header (Like the one from ngrep)
    char msg_header[256];
    // Packet payload data
    u_char *msg_payload;
    // Packet payload size
    int size_payload;

    // Open PCAP file
    if ((handle = pcap_open_offline(file, errbuf)) == NULL) {
        fprintf(stderr, "Couldn't open pcap file %s: %s\n", file, errbuf);
        return 1;
    }

    // Get datalink to parse packages correctly
    linktype = pcap_datalink(handle);

    // Loop through packages
    while ((packet = pcap_next(handle, &header))) {
        // Get link header size from datalink type
        if (linktype == DLT_EN10MB) {
            eptr = (struct ether_header *) packet;
            if ((ether_type = ntohs(eptr->ether_type)) != ETHERTYPE_IP) continue;
            size_link = SIZE_ETHERNET;
        } else if (linktype == DLT_LINUX_SLL) {
            size_link = SLL_HDR_LEN;
        } else {
            // Something we are not prepared to parse :(
            fprintf(stderr, "Error handing linktype %d\n", linktype);
            return 1;
        }

        // Get IP header 
        ip = (struct nread_ip*) (packet + size_link);
        size_ip = IP_HL(ip) * 4;
        // Only interested in UDP packets
        if (ip->ip_p != IPPROTO_UDP) continue;

        // Get UDP header
        udp = (struct nread_udp*) (packet + size_link + size_ip);

        // Get package payload
        msg_payload = (u_char *) (packet + size_link + size_ip + SIZE_UDP);
        size_payload = htons(udp->udp_hlen) - SIZE_UDP;
        msg_payload[size_payload] = '\0';

        // XXX Process timestamp
        struct timeval ut_tv = header.ts;
        time_t t = (time_t) ut_tv.tv_sec;

        // XXX Get current time 
        char timestr[200];
        struct tm *time = localtime(&t);
        strftime(timestr, sizeof(timestr), "%Y/%m/%d %T", time);

        // XXX Build a header string
        sprintf(msg_header, "U %s.%06ld", timestr, ut_tv.tv_usec);
        sprintf(msg_header, "%s %s:%u", msg_header, inet_ntoa(ip->ip_src), htons(udp->udp_sport));
        sprintf(msg_header, "%s -> %s:%u", msg_header, inet_ntoa(ip->ip_dst), htons(udp->udp_dport));

        // Parse this header and payload
        sip_load_message(msg_header, (const char*) msg_payload);
    }

    // Close PCAP file
    pcap_close(handle);
    return 0;
}
