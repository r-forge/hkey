\name{hkey}
\alias{hkey}
\alias{hkey}
\alias{hkey-package}
\alias{as.list.hkey}
\alias{names.hkey}
\alias{print.hkey}
\alias{close.hkey}
\alias{[[.hkey}
\alias{$.hkey}
\alias{[.hkey}
\alias{[[<-.hkey}
\alias{$<-.hkey}
\alias{[<-.hkey}
\alias{hkey.subkey.suffix}
\alias{hkey.default.valname}
\alias{hkey.expand.regex}
\alias{hkey.hkey.regex}
\alias{hkey.types}

%- Also NEED an '\alias' for EACH other topic documented here.
\title{hkey - read and write the Windows registry}
\description{
  This allows creation, reading, modifying, and deleting windows
  registry nodes and their values.  The interface uses methods for
  the usual R list indexing and assignment operators.  Objects are
  converted in a sensible way between native R types and those
  of the registry.
}
\usage{
hkey(name = "HKEY_LOCAL_MACHINE")
\method{print}{hkey}(x, ...)
\method{as.list}{hkey}(x, ...)
\method{names}{hkey}(x)
\method{close}{hkey}(con, ...)
x[[index, use.value=FALSE]]
x$INDEX
x[indexlist]
x[[index, use.value=FALSE]] <- value
x$INDEX <- value
x[indexlist] <- value

hkey.subkey.suffix
hkey.default.valname
hkey.expand.regex
hkey.hkey.regex
hkey.types

}
\arguments{
  \item{name}{a character scalar giving a registry node name}
  \item{x}{an object of class \code{hkey}}
  \item{...}{extra arguments to \code{print}, or for \code{as.list}}
  \item{index}{a character string naming a value or subnode of node
    \code{x}; the default i.e. unnamed value for a node is referred to with an
    empty string index (i.e. \code{""}) or the value of
    \code{hkey.default.valname}}
  
  \item{con}{an hkey object to be closed. The odd parameter name is
  for consistency with R generic method rules.}
  \item{INDEX}{a quoted or unquoted character constant naming a value or
    subnode of a node}
  \item{indexlist}{a character vector naming values or subnodes of a
    node;  a missing \code{indexlist} represents all possible indexes.}
  \item{value}{a new value for a new or existing key value or subkey}

  \item{use.value}{when \code{index} is the name of both a value and a
    subkey, setting \code{use.value=TRUE} makes this operation refer to
    the value, rather than the default of refering to the subnode.}
    
  \item{hkey.subkey.suffix}{character scalar appended to subnode names
  to distinguish them from value names}
  \item{hkey.default.valname}{name used to represent the unnamed
    default node value.  This avoids the problem of R not allowing an
    empty string as an index.}
  \item{hkey.expand.regex}{a regular expression for recognizing
    strings which should be converted to registry values of type REG_EXPAND_SZ instead of
    REG_SZ; these are meant to contain unexpanded references to
    environment variables, such as \code{\%PATH\%}}
  \item{hkey.hkey.regex}{a regular expression for recognizing strings
    which should be converted to registry values of type REG_LINK
    instead of REG_SZ (WARNING: untested)}
  \item{hkey.types}{a named list of values for Windows registry value
    types; these correspond to the \code{REG_...} constants}
}
\details{
  Names in a node's key path are delimited by forward slashes ("/")
  rather than the backward slashes ("\\") typical in Windows.
  
  Values are converted back and forth between R native types and Windows
  registry types according to this table:
  
       \tabular{cccl}{                            
         R class \tab Length   \tab Matching \tab Registry class \cr
	 character \tab 1      \tab hkey.expand.regex \tab REG_EXPAND_SZ \cr
	 character \tab 1      \tab hkey.hkey.regex \tab REG_LINK \cr
	 character \tab 1      \tab (neither) \tab REG_SZ \cr
	 character \tab > 1    \tab (neither) \tab MULTI_SZ \cr
	 integer \tab 1      \tab  \tab REG_DWORD \cr
	 integer \tab 2      \tab  \tab REG_QWORD \cr
	 raw \tab n \tab \tab REG_BINARY \cr
	 list \tab \tab \tab (sub node) \cr
       }

  The value assigned to a subnode must be a list with named elements of the
  type in the table above, or named elements which are themselves lists,
  in which case the latter becomes a subnode of the subnode.  Nesting of
  lists (subnodes) to any level is permitted.

  Items of class "hkey" are EXTPTRs to Windows HKEYS, i.e. handles of
  "open" registry nodes, and with attributes:
  \item{key}{character scalar giving the the full node path}
  \item{children}{character vector giving the names of subnodes}
  \item{values}{environment in which values of the node have been
    assigned to their names.  The default value is assigned to a symbol
    with the name given by \code{hkey.default.valname}}
  
  Finalizers are registered for "hkey" objects so that the Windows HKEYs
  will be closed when the items are no longer in use.  To do this immediately,
  use \code{close(x)}., which sets the \code{class} attribute of \code{x} to \code{NULL}, to prevent its inadvertent use.

  To distinguish between names of subnodes and names of values, the
  former are suffixed with hkey.subkey.suffix, which is also stripped
  from user-supplied \code{index} values.  So if a node has a subnode
  and a value with the same name, the former will be used in
  assignments.  To force the use of a value, use the
  \code{use.value} flag in indexing and assignment.

  To cause \code{as.list(x)} to preserve classes of values copied from
  the registry, supply the \code{unclass} argument as so:
  \code{as.list(x, unclass=FALSE)}.
}
\value{
  \code{hkey} returns an "hkey" object corresponding to the node
  \code{name}, if it exists, or \code{NULL} if it does not

  Indexing an "hkey" object returns either another "hkey" object (if the
  index refers to a subnode) or a native R value with class attribute
  "REG_..." from the table above.

  Indexed assignment to an "hkey" object creates, modifies, or deletes
  a subnode or value, depending on whether the index already exists,
  and on whether the value being assigned is \code{NULL}.

  \code{as.list} recursively descends from a node to all of its
  descendents, building a list whose names are names of keys and values,
  and whose values are native R values (without the \code{REG_...} class
  attributes) or sublists (in the case of subnodes).  WARNING:  This can generate
  a very large structure if used with an upper-level key.

  To get an unexpanded list of just the children of \code{x}, use
  \code{x[]}.  There, values will be represented by their native R
  equivalents with the \code{REG_...} class attributes, and subkeys will
  be represented as (unexpanded) objects of class "hkey".
  
  \code{print} displays the name of the full key path.
}
\author{John Brzustowski <jbrzusto@fastmail.fm}
\examples{

library(hkey)
hkey()
names(hkey())
x <- hkey("HKEY_LOCAL_MACHINE/SOFTWARE")
names(x)
x <- x$"R-core"
names(x)
x <- x[["R"]]
x[]
as.list(x)
x$new.value <- "This will be ignored by R, I hope"
x$new.subnode <- list(a=12345, b=c("one","more","time"),
d=as.raw(10:1),e=list(nesting=c(2,2)))
as.list(x)
x[c("new.value", "new.subnode")] <- list(NULL, NULL)


## Win32 registry node: HKEY_LOCAL_MACHINE
## Use names() to list subkeys and data, $, [], [[]] to extract them.


}
\keyword{ environment }
\keyword{ utilities }
\keyword{ misc }
