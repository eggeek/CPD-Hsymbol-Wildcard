loadData <- function(x) {
  data <- read.csv(x)
  return (data)
}

plotFirstMove <- function(path) {
  d = loadData(path)
  myColors = c("red", "blue", "darkgoldenrod2", "darkviolet","hotpink", "green4", "lightpink3", "mediumorchid4", "dimgrey")
  colors = myColors[d$eid + 1]
  plot(d$latt, d$lont, col=colors, pch='.')

  points(d[1,]$lats, d[1,]$lons, col="black")
}

plotTargets <- function(path) {
  d = loadData(path)
  plot(d$latt, d$lont, col="red", pch='.')
  points(d[1,]$lats, d[1,]$lons, col="blue")
}

int2rgb <- function(int) {
  r = bitwAnd(bitwShiftR(int, 16), 0XFF)
  g = bitwAnd(bitwShiftR(int, 8), 0XFF)
  b = bitwAnd(bitwShiftR(int, 0), 0XFF)
  return (rgb(r/255, g/255, b/255))
}

plotRow <- function(path) {
  d = loadData(path)
  colors = int2rgb(d$color);
  plot(d$latt, d$lont, col=colors, pch='.')
  points(d[1,]$lats, d[1,]$lons, col="black");
}

plotLoad <- function(path) {
  d = loadData(path)
  plot(d$latt, d$lont, ylim=rev(range(d$lont)), col=d$hex, pch=d$pch)
  points(d[1,]$lats, d[1,]$lons, ylim=rev(range(d$lont)), col="black", pch="o")
}
