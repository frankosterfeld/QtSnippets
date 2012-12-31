QtSnippets
==========

QtSnippets is a collection of independent Qt helper classes I find useful but are too small to deserve their own library.

It contains:

 * DelayingProxyDevice: A QIODevice proxy ensuring the data to arrive in small chunks instead of all at once (the simple case). 
Meant to unit-test parsers and other code reading from I/O devices that must be able to handle partial reads.

**License:** QtSnippets is available under the [Modified BSD License](http://www.gnu.org/licenses/license-list.html#ModifiedBSD). See the file COPYING for details.
