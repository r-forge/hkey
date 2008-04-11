`hkey.fillin.hkey` <-
function (x, key = attr(x, "key")) 
{
    attr(x, "key") <- key
    attr(x, "children") <- .Call("get_subnode_names", x)
    attr(x, "values") <- .Call("get_node_values", x, new.env(emptyenv()), 
        hkey.default.valname)
    reg.finalizer(x, hkey.close.hkey, TRUE)
    return(x)
}
