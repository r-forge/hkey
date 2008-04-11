`[[.hkey` <-
function (x, i, use.value=FALSE) 
{
  if (i == "")
    i <- hkey.default.valname
  if (!use.value) {
    ## give priority to subnodes over values (in case i is a name shared by both)
    i <- hkey.clean.indexes(i)
    if (i %in% attr(x, "children")) 
      return(hkey.fillin.hkey(.Call("get_subnode", x, i), 
                                paste(attr(x, "key"), i, sep = "/")))
  }
  return(attr(x, "values")[[i]])
}
