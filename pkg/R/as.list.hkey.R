`as.list.hkey` <-
function (x, ...) 
{
  extra <- list(...)
  unclass <- is.null(extra$unclass) || isTRUE(extra$unclass)
    lapply(x[], function(i) {
      if (inherits(i, "hkey")) 
        as.list(i)
      else if (unclass) 
        unclass(i)
      else i
      })
}
