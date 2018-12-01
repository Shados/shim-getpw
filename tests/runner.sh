#!/usr/bin/env bash
SHIM=$1
TEST_STR_BIN=$2
TEST_ID_BIN=$3

RET=0
NSS_HOME=$(getent passwd $UID | cut -d : -f 6)
NSS_GID=$(getent passwd $UID | cut -d : -f 4)

echo -n "Testing home directory without shim... "
RESULT_NOSHIM=$($TEST_STR_BIN)
if [ "$RESULT_NOSHIM" != "$NSS_HOME" ]; then
  echo "Failed!"
  echo "RESULT_NOSHIM ($RESULT_NOSHIM) != NSS_HOME ($NSS_HOME)"
  RET=1
else
  echo "OK"
fi

echo -n "Testing home directory with shim but without variable... "
RESULT_SHIM=$(LD_PRELOAD=$SHIM $TEST_STR_BIN)
if [ "$RESULT_SHIM" != "$NSS_HOME" ]; then
  echo "Failed!"
  echo "RESULT_SHIM ($RESULT_SHIM) != NSS_HOME ($NSS_HOME)"
  RET=1
else
  echo "OK"
fi

echo -n "Testing home directory with shim and variable... "
export SHIM_HOME=$PWD
RESULT_SHIM=$(LD_PRELOAD=$SHIM $TEST_STR_BIN)
if [ "$RESULT_SHIM" != "$SHIM_HOME" ]; then
  echo "Failed!"
  echo "RESULT_SHIM ($RESULT_SHIM) != SHIM_HOME ($SHIM_HOME)"
  RET=1
else
  echo "OK"
fi

echo -n "Testing gid without shim... "
RESULT_NOSHIM=$($TEST_ID_BIN)
stat=$?
if [ "$stat" != 0 ]; then
  echo "$TEST_ID_BIN" failed to run!
fi
if [ "$RESULT_NOSHIM" != "$NSS_GID" ]; then
  echo "Failed!"
  echo "RESULT_NOSHIM ($RESULT_NOSHIM) != NSS_GID ($NSS_GID)"
  RET=1
else
  echo "OK"
fi

echo -n "Testing gid with shim but without variable... "
RESULT_SHIM=$(LD_PRELOAD=$SHIM $TEST_ID_BIN)
stat=$?
if [ "$stat" != 0 ]; then
  echo "$TEST_ID_BIN" failed to run!
fi
if [ "$RESULT_SHIM" != "$NSS_GID" ]; then
  echo "Failed!"
  echo "RESULT_SHIM ($RESULT_SHIM) != NSS_GID ($NSS_GID)"
  RET=1
else
  echo "OK"
fi

echo $USER
echo -n "Testing gid with shim and variable... "
export SHIM_GID=54201
RESULT_SHIM=$(LD_PRELOAD=$SHIM $TEST_ID_BIN)
stat=$?
if [ "$stat" != 0 ]; then
  echo "$TEST_ID_BIN" failed to run!
fi
if [ "$RESULT_SHIM" != "$SHIM_GID" ]; then
  echo "Failed!"
  echo "RESULT_SHIM ($RESULT_SHIM) != SHIM_GID ($SHIM_GID)"
  RET=1
else
  echo "OK"
fi

exit $RET
