/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009-2010 Enrico Ros
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#if defined(TIMING_USE_ITIMER)
 #include <signal.h>
#elif defined(TIMING_USE_TASKSTATS)
 #include <errno.h>
 #include <linux/genetlink.h>
 #include <linux/taskstats.h>
 #include <linux/cgroupstats.h>
#endif

namespace Inspector {
namespace Probe {
namespace TimeCounter {

#if defined(TIMING_USE_ITIMER)
    static unsigned int s_ipSigProfCounter = 0;
    static void ipSigProf(int)
    {
        ++s_ipSigProfCounter;
        CONSOLE_PRINT("SP %d", s_ipSigProfCounter);
    }

    static double elapsedMs()
    {
        return (double)s_ipSigProfCounter / 1000.0;
    }

    static void startTimer()
    {
        s_ipSigProfCounter = 0;
        struct itimerval tim;
        tim.it_interval.tv_sec = 0;
        tim.it_interval.tv_usec = 1;
        tim.it_value.tv_sec = 0;
        tim.it_value.tv_usec = 1;
        setitimer(ITIMER_PROF, &tim, 0);
    }

    static void stopTimer()
    {
        setitimer(ITIMER_PROF, 0, 0);
    }

    static void init()
    {
        CONSOLE_PRINT("init");
        struct sigaction sa;
        sa.sa_handler = ipSigProf;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGPROF, &sa, 0) != 0) {
            CONSOLE_PRINT("Failed to register signal handler.");
            exit(-1);
        }
    }

    static void close()
    {
        // TODO: deregister signal
    }
#elif defined(TIMING_USE_TASKSTATS)
    /*
     * Generic macros for dealing with netlink sockets. Might be duplicated
     * elsewhere. It is recommended that commercial grade applications use
     * libnl or libnetlink and use the interfaces provided by the library
     */
    #define GENLMSG_DATA(glh)	((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
    #define GENLMSG_PAYLOAD(glh)	(NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
    #define NLA_DATA(na)		((void *)((char*)(na) + NLA_HDRLEN))
    #define NLA_PAYLOAD(len)	(len - NLA_HDRLEN)

    // Maximum size of response requested or message sent
    #define MAX_MSG_SIZE 1024
    struct NlMsgTemplate {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[MAX_MSG_SIZE];
    };

    // netlink: create a comm socket
    static int netlink_createSocket(int protocol)
    {
        int fd = socket(AF_NETLINK, SOCK_RAW, protocol);
        if (fd < 0)
            return -1;
        struct sockaddr_nl local;
        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
            ::close(fd);
            return -1;
        }
        return fd;
    }

    // netlink: send command
    int netlink_sendCmd(int socket, __u16 nlmsg_type, __u32 nlmsg_pid,
             __u8 genl_cmd, __u16 nla_type,
             void *nla_data, int nla_len)
    {
        struct NlMsgTemplate msg;
        msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
        msg.n.nlmsg_type = nlmsg_type;
        msg.n.nlmsg_flags = NLM_F_REQUEST;
        msg.n.nlmsg_seq = 0;
        msg.n.nlmsg_pid = nlmsg_pid;
        msg.g.cmd = genl_cmd;
        msg.g.version = 0x1;

        struct nlattr *na = (struct nlattr *)GENLMSG_DATA(&msg);
        na->nla_type = nla_type;
        na->nla_len = nla_len + 1 + NLA_HDRLEN;
        memcpy(NLA_DATA(na), nla_data, nla_len);
        msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        char *buf = (char *)&msg;
        int buflen = msg.n.nlmsg_len;
        struct sockaddr_nl nladdr;
        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;
        int r;
        while ((r = sendto(socket, buf, buflen, 0, (struct sockaddr *) &nladdr,
                   sizeof(nladdr))) < buflen) {
            if (r > 0) {
                buf += r;
                buflen -= r;
            } else if (errno != EAGAIN)
                return -1;
        }
        return 0;
    }

    // netlink: get family id
    static int netlink_getFamilyId(int socket)
    {
        struct {
            struct nlmsghdr n;
            struct genlmsghdr g;
            char buf[256];
        } ans;

        char name[100];
        strcpy(name, TASKSTATS_GENL_NAME);
        netlink_sendCmd(socket, GENL_ID_CTRL, getpid(), CTRL_CMD_GETFAMILY,
                CTRL_ATTR_FAMILY_NAME, (void *)name,
                strlen(TASKSTATS_GENL_NAME)+1);

        int rep_len = recv(socket, &ans, sizeof(ans), 0);
        if (ans.n.nlmsg_type == NLMSG_ERROR || (rep_len < 0) || !NLMSG_OK((&ans.n), rep_len))
            return 0;

        struct nlattr *na = (struct nlattr *) GENLMSG_DATA(&ans);
        na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
        if (na->nla_type == CTRL_ATTR_FAMILY_ID)
            return *(__u16 *)NLA_DATA(na);
        return 0;
    }

    static int netlinkSocket = 0;
    static int netlinkFamilyId = 0;
    static pid_t netlinkPid = 0;
    static struct NlMsgTemplate receivedMsg;

    static __u64 getProcessCpuNs()
    {
        if (!netlinkSocket) {
            CONSOLE_PRINT("not linked");
            return 0;
        }

        // request task stats
        if (netlink_sendCmd(netlinkSocket, netlinkFamilyId, netlinkPid, TASKSTATS_CMD_GET, TASKSTATS_CMD_ATTR_PID, &netlinkPid, sizeof(__u32)) < 0) {
            CONSOLE_PRINT("error sending tid/tgid cmd");
            return 0;
        }

        // read reply
        int replySize = recv(netlinkSocket, &receivedMsg, sizeof(receivedMsg), 0);
        if (replySize < 0) {
            CONSOLE_PRINT("nonfatal reply error: errno %d", errno);
            return 0;
        }

        // check error
        if (receivedMsg.n.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&receivedMsg.n), replySize)) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(&receivedMsg);
            CONSOLE_PRINT("fatal reply error, errno %d", err->error);
            return 0;
        }

        // unroll and look into payload for PID STATS
        int payloadSize = GENLMSG_PAYLOAD(&receivedMsg.n);
        struct nlattr *na = (struct nlattr *)GENLMSG_DATA(&receivedMsg);
        int idx = 0;
        while (idx < payloadSize) {
            idx += NLA_ALIGN(na->nla_len);
            if (na->nla_type == TASKSTATS_TYPE_AGGR_PID) {
                int subSize = NLA_PAYLOAD(na->nla_len);
                int idx2 = 0;
                na = (struct nlattr *)NLA_DATA(na);
                while (idx2 < subSize) {
                    if (na->nla_type == TASKSTATS_TYPE_STATS) {
                        struct taskstats *ts = (struct taskstats *)NLA_DATA(na);
                        return ts->cpu_run_real_total;
                    }
                    idx2 += NLA_ALIGN(na->nla_len);
                    na = (struct nlattr *)((char *) na + idx2);
                }
            }
            na = (struct nlattr *)(GENLMSG_DATA(&receivedMsg) + idx);
        }
        return 0;
    }

    static __u64 startTime;

    static double elapsedMs()
    {
        // workaround __u64 wraparound
        __u64 stopTime = getProcessCpuNs();
        if (stopTime < startTime)
            return stopTime / 1000000.0;
        return (double)(stopTime - startTime) * 0.000001;
    }

    static void startTimer()
    {
        startTime = getProcessCpuNs();
    }

    static void stopTimer()
    {
    }

    static void close()
    {
        if (netlinkSocket) {
            ::close(netlinkSocket);
            netlinkSocket = 0;
        }
    }

    static void init()
    {
        if (netlinkSocket) {
            CONSOLE_PRINT("Netlink already started");
            return;
        }
        if ((netlinkSocket = netlink_createSocket(NETLINK_GENERIC)) < 0) {
            CONSOLE_PRINT("error creating Netlink socket");
            return;
        }

        netlinkFamilyId = netlink_getFamilyId(netlinkSocket);
        if (!netlinkFamilyId) {
            CONSOLE_PRINT("Error getting family id, errno %d", errno);
            close();
            return;
        }

        netlinkPid = getpid();
    }
#endif
} // namespace TimeCounter
} // namespace Probe
} // namespace Inspector
