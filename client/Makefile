#
# Makefile for osd2web client
#

cbuild: client

client:
	if ! test -d src/custom; then \
	   mkdir src/custom; \
	   chmod a+rx src/custom; \
	fi
	npm run build
