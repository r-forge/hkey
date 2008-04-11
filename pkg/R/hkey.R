`hkey` <-
function (name = "HKEY_LOCAL_MACHINE") 
{
    if (!is.null(rv <- .Call("get_node", name))) 
        rv <- hkey.fillin.hkey(rv, name)
    return(rv)
}
