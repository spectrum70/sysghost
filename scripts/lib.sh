# sysghost library

success()
{
	echo -e "\x1b[92mdone\x1b[0m"
}

error()
{
	echo -e "\x1b[91merror\x1b[0m"
}

step()
{
	echo -e -n "\x1b[92m● \x1b[0m${1}"
}

emsg()
{
	echo -e "\x1b[31;1m* ${1}\x1b[0m"
}

start_service()
{
	# start_service must be called with double quotes "command"
	step "daemon: starting ${1} ... "
	eval $@
	if [ $? == 0 ]; then
		success
	else
		error
	fi
}
