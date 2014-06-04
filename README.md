*procctrl* can be used to start a process, and later stop it, by referencing
it symbolically instead of by process id. Additionally, a "parent" process can
be monitored and the spawned process killed when that parent terminates.

This is typically used for integration test harnesses using, for example,
Maven or Ant. If a server process required for the test can be started using
this utility then it will be killed whenever the parent build/test process
terminates. This avoids the problem of rogue processes remaining after failed
or aborted builds/tests.

Example usage with Ant
----------------------

~~~{.xml}
<property name="procctrl.cmd" value="procctrl" />
<property name="test.server.id" value="IntegrationTestServer" />
<target name="pre-integration-test">
  <exec executable="${procctrl.cmd}">
    <arg value="-k${test.server.id}" />
    <arg value="-p" />
    <arg value="start" />
    <arg value="run-my-server-command" />
    <arg value="my-server-parameter" />
  </exec>
</target>
<target name="post-integration-test">
  <exec executable="${procctrl.cmd}">
    <arg value="-k${test.server.id}" />
    <arg value="stop" />
  </exec>
</target>
<target name="integration-test-run">
  <!-- run the test, requiring the server process to be running -->
</target>
<target name="integration-test" depends="pre-integration-test, integration-test-run, post-integration-test" />
~~~

The *integration-test* target in the above example starts an external server
process needed by the test and makes sure it terminates afterwards. The
**k** parameter specifies a logical name for the server process that is used
to identify the process when it must be stopped after the test.

If the test fails and the Ant process terminates before the
*post-integration-test* target is reached then the **p** parameter will
ensure that the server process gets killed as Ant terminates.

Building from source
--------------------

To build from source on Linux, or similar, platforms use the following in the
top-level directory (that is, the one containing this file).

  * autoconf
  * automake
  * autoheader
  * ./configure
  * make check

To install the *procctrl* utility, use `make install` instead of `make check`.
This must typically be run as `root` to write to the system folders.
