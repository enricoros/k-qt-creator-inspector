<plugin name="Performance" version="1.2.80" compatVersion="1.2.80">
    <vendor>Enrico Ros</vendor>
    <copyright>(C) 2009 Enrico Ros</copyright>
    <license>
Commercial Usage

Licensees holding valid Qt Commercial licenses may use this plugin in
accordance with the Qt Commercial License Agreement provided with the
Software or, alternatively, in accordance with the terms contained in
a written agreement between you and Nokia.

GNU Lesser General Public License Usage

Alternatively, this plugin may be used under the terms of the GNU Lesser
General Public License version 2.1 as published by the Free Software
Foundation.  Please review the following information to
ensure the GNU Lesser General Public License version 2.1 requirements
will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.</license>
    <description>Performance Monitoring plugin.</description>
    <url>http://enricoros.wordpress.com</url>
    <dependencyList>
        <dependency name="Core" version="1.2.80"/>
        <dependency name="Debugger" version="1.2.80"/>
    </dependencyList>
    <argumentList>
        <argument name="-disable-slotmon">Disable Signal/Slot monitoring</argument>
        <argument name="-disable-eloop">Disable Event loop statistics</argument>
        <argument name="-disable-mmon">Disable Memory allocations monitoring</argument>
    </argumentList>
</plugin>
