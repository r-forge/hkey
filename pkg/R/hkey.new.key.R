`hkey.new.key` <-
function (x, subkey, value) 
{
    sk <- .Call("create_subnode", x, subkey)
    n <- names(value)
    if (length(n) == 0) {
        n <- ""
        if (length(value) > 1) 
            stop("Cannot assign multiple item to the default (unnamed) value")
    }
    for (i in seq(along = value)) {
        v <- value[[i]]
        if (is.list(v)) {
            if (n[[1]] == "") 
                stop("Cannot assign a structure to an unnamed subkey")
            hkey.new.key(sk, n[[i]], v)
        }
        else {
            hkey.assign.value(sk, n[[i]], v)
        }
    }
}
