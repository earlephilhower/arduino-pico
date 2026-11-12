#!/usr/bin/env python3

# Generates wrapper files from a C header to use in lwip_wrap/freertos-lwip to simplify lives
# Need to manually add brackets around #if's functions which are disabled in wrap_..._cases.inc
#

import json
import sys
import argparse
import subprocess
import re

parser = argparse.ArgumentParser(description='Wrapper file generator')
parser.add_argument('-s', '--source', action='store', required=True, help='source header file')
parser.add_argument('-p', '--prefix', action='store', required=True, help='output file prefix')
parser.add_argument('-m', '--mutex', action='store', required=False, default='LWIPMutex', help='mutex class for event handler')
parser.add_argument('-t', '--threadcheck', action='store', required=False, default='__isLWIPThread', help='function returns is thread is worker thread')
parser.add_argument('-q', '--messagequeue', action='store', required=False, default='__lwip', help='freertos message queue function')

args = parser.parse_args()

# Overrides we already know about, if the fcn name matches we dump the FreeRTOS case in a if-block
ifdefs = {}
ifdefs['pbuf_alloced_custom'] = "#if LWIP_SUPPORT_CUSTOM_PBUF"
ifdefs['pbuf_fill_chksum'] = "#if LWIP_CHECKSUM_ON_COPY"
ifdefs['pbuf_split_64k'] = "#if LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE"
ifdefs['lwip_tcp_event'] = "#if LWIP_EVENT_API"
ifdefs['netif_add_ext_callback'] = "#if LWIP_NETIF_EXT_STATUS_CALLBACK"
ifdefs['netif_remove_ext_callback'] = "#if LWIP_NETIF_EXT_STATUS_CALLBACK"
ifdefs['netif_invoke_ext_callback'] = "#if LWIP_NETIF_EXT_STATUS_CALLBACK"
ifdefs['netif_ip6_addr_set'] = "#if LWIP_IPV6"
ifdefs['netif_ip6_addr_set_parts'] = "#if LWIP_IPV6"
ifdefs['netif_ip6_addr_set_state'] = "#if LWIP_IPV6"
ifdefs['netif_get_ip6_addr_match'] = "#if LWIP_IPV6"
ifdefs['netif_create_ip6_linklocal_address'] = "#if LWIP_IPV6"
ifdefs['netif_add_ip6_address'] = "#if LWIP_IPV6"
ifdefs['tcp_ext_arg_alloc_id'] = "#if LWIP_TCP_PCB_NUM_EXT_ARGS"
ifdefs['tcp_ext_arg_set_callbacks'] = "#if LWIP_TCP_PCB_NUM_EXT_ARGS"
ifdefs['tcp_ext_arg_set'] = "#if LWIP_TCP_PCB_NUM_EXT_ARGS"
ifdefs['tcp_ext_arg_get'] = "#if LWIP_TCP_PCB_NUM_EXT_ARGS"
ifdefs['mld6_stop'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_report_groups'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_tmr'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_input'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_joingroup'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_joingroup_netif'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_leavegroup'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_leavegroup_netif'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"
ifdefs['mld6_lookfor_group'] = "#if LWIP_IPV6 && LWIP_IPV6_MLD"

ifdefs['udp_sendto_if_chksum'] = "#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP"
ifdefs['udp_sendto_chksum'] = "#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP"
ifdefs['udp_sendto_if_chksum'] = "#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP"
ifdefs['udp_send_chksum'] = "#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP"
ifdefs['udp_sendto_if_src_chksum'] = "#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP"
ifdefs['udp_debug_print'] = "#if UDP_DEBUG"
ifdefs['dns_local_iterate'] = "#if DNS_LOCAL_HOSTLIST"
ifdefs['dns_local_lookup'] = "#if DNS_LOCAL_HOSTLIST"
ifdefs['dns_local_removehost'] = "#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC"
ifdefs['dns_local_addhost'] = "#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC"
ifdefs['sntp_servermode_dhcp'] = "#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6"
ifdefs['dhcp_set_ntp_servers'] = "#if LWIP_DHCP && SNTP_GET_SERVERS_FROM_DHCP"
ifdefs['netif_get_loopif'] = "#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF"
ifdefs['netif_loop_output'] = "#if ENABLE_LOOPBACK"
ifdefs['netif_poll'] = "#if ENABLE_LOOPBACK"
ifdefs['netif_poll_all'] = "#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING"
ifdefs['netif_set_remove_callback'] = "#if LWIP_NETIF_REMOVE_CALLBACK"
ifdefs['sys_timeouts_get_next_timeout'] = "#if LWIP_TESTMODE"
ifdefs['lwip_cyclic_timer'] = "#if LWIP_TESTMODE"
ifdefs['sys_timeout_debug'] = "#if LWIP_DEBUG_TIMERNAMES"
ifdefs['sys_timeout'] = "#if !LWIP_DEBUG_TIMERNAMES"

ifdefs['lfs_migrate'] = "#ifdef LFS_MIGRATE"

# ctags entries which appear but should be skipped
skips = {}
skips['TCP_PCB_COMMON'] = 1

skipfirst = {}
skipfirst['netif_add'] = 1

enumtypes = []
externdefs = []
structs = []
funcdefs = []
frees = []
links = []

def wrapifdef(func, item):
    if func in ifdefs:
        return ifdefs[func] + "\n" + item + "\n" + "#endif"
    else:
        return item


ctags = subprocess.Popen(["ctags", "--pattern-length-limit=0", "--fields=+S+N", "--output-format=json", "--c-kinds=pf", args.source], stdout=subprocess.PIPE)
for line in ctags.stdout.readlines():
    api = json.loads(line)
    apiline = api['pattern'][2:]
    apiline = apiline[:-2]

    externdef = ""
    struct = ""
    funcdef = ""
    free = ""

    try:
        # Make (void) signatures into ()
        api['signature'] = re.sub(  r'^\s?\(\s?void\s?\)\s?$', "()", api['signature'])

        params = api['signature'][1:-1].split(",")

        func = api['name']

        if func in skipfirst:
            del skipfirst[func]
            continue

        if func in skips:
            continue

        links = links + ["-Wl,--wrap=" + func]

        functype = api['typeref'].replace(":", " ").replace("typename ", "")

        enumtype = "__" + func + ",";
        enumtype = wrapifdef(func, enumtype)

        externdef = "extern " + functype + " __real_" + func + api['signature'] + ";"
        externdef = wrapifdef(func, externdef)

        struct = "typedef struct {\n";
        for param in params:
            struct = struct + "    " + param + ";\n"
        if not functype == "void":
            struct = struct + "    " + functype + " *ret;\n"
        struct = struct + "} __" + func + "_req;"
        struct = wrapifdef(func, struct)


        funcdef = functype + " __wrap_" + func +  api['signature'] + " {\n"
        funcdef = funcdef + "#ifdef __FREERTOS\n"
        funcdef = funcdef + "    if (!" + args.threadcheck + "()) {\n"
        if not functype == "void":
            funcdef = funcdef + "        " + functype + " ret;\n"
        funcdef = funcdef + "        __" + func + "_req req = { "
        elems = []
        for p in params:
            spaceidx = p.rfind(' ')
            refidx = p.rfind('*')
            if spaceidx > refidx:
                elem = p[spaceidx + 1:].strip()
            else:
                elem = p[refidx + 1:].strip()
            if elem != "":
                elems = elems + [elem]
        elemsnoret = elems
        if not functype == "void":
            elems = elems + ["&ret"]
        funcdef = funcdef + ", ".join(elems) + " };\n"
        funcdef = funcdef + "        " + args.messagequeue + "(__" + func + ", &req);\n"
        if not functype == "void":
            funcdef = funcdef + "        return ret;\n"
        else:
            funcdef = funcdef + "        return;\n"
        funcdef = funcdef + "    }\n"
        funcdef = funcdef + "#endif\n"
        funcdef = funcdef + "    " + args.mutex + " m;\n"
        if not functype == "void":
            funcdef = funcdef + "    return __real_" + func + "(" + ", ".join(elemsnoret) + ");\n"
        else:
            funcdef = funcdef + "    __real_" + func + "(" + ", ".join(elemsnoret) + ");\n"
        funcdef = funcdef + "}\n"
        funcdef = wrapifdef(func, funcdef)

        free = "case __" + func + ": {\n";
        if len(elems) > 0:
            free = free + "    __" + func + "_req *r = (__" + func + "_req *)w.req;\n"
        if not functype == "void":
            free = free + "    *(r->ret) = "
        else:
            free = free + "    "
        for i in range(0, len(elemsnoret)):
            elemsnoret[i] = "r->" + elemsnoret[i]
        free = free + "__real_" + func + "(" + ", ".join(elemsnoret) + ");\n"
        free = free + "    break;\n"
        free = free + "}"
        free = wrapifdef(func, free)

    except:
        externdef = "FAIL: " + apiline
        struct = externdef
        funcdef = externdef
        free = externdef

    enumtypes = enumtypes + [enumtype]
    externdefs = externdefs + [externdef]
    structs = structs + [struct]
    funcdefs = funcdefs + [funcdef]
    frees = frees + [free]

with open(args.prefix + "_enums.inc", "w") as f:
    f.write("// Machine generated (makewrapper.py).  DO NOT EDIT\n");
    f.write("\n".join(enumtypes) + "\n")

with open(args.prefix + "_externs.inc", "w") as f:
    f.write("// Machine generated (makewrapper.py).  DO NOT EDIT\n");
    f.write("\n".join(externdefs) + "\n")

with open(args.prefix + "_structs.inc", "w") as f:
    f.write("// Machine generated (makewrapper.py).  DO NOT EDIT\n");
    f.write("\n".join(structs) + "\n")

with open(args.prefix + "_functions.inc", "w") as f:
    f.write("// Machine generated (makewrapper.py).  DO NOT EDIT\n");
    f.write("\n".join(funcdefs) + "\n")

with open(args.prefix + "_cases.inc", "w") as f:
    f.write("// Machine generated (makewrapper.py).  DO NOT EDIT\n");
    f.write("\n".join(frees) + "\n")

with open(args.prefix + "_wraps.inc", "w") as f:
    f.write("\n".join(links) + "\n")
