#PBI Homework NO. 1
#@author: Filip Gulan
#@mai: xgulan00@stud.fit.vutbr.cz
#@date: 9.11.2017

#Repos
source("https://bioconductor.org/biocLite.R")
biocLite()
biocLite("biomaRt")
biocLite("dplyr")
biocLite("Gviz")
biocLite("GenomicRanges")

#Libraries
library(biomaRt)
library(ggplot2) 
library(dplyr)
library(GenomicRanges)
library(Gviz)
ensembl=useMart("ensembl")

#Get human HOX genes
#ensembl = useDataset("hsapiens_gene_ensembl",mart=ensembl)
#atributes = c('chromosome_name', 'start_position', 'end_position', 'transcript_start', 'transcript_end', 'external_gene_name', 'hgnc_symbol')
#genes = getBM(attributes = atributes, mart = ensembl)
#tempHoxGenes = genes[grepl("HOX", genes$hgnc_symbol), ]
tempHoxGenes = read.table("hgTables.txt", header=TRUE, sep="\t", na.strings="?", stringsAsFactors=FALSE)
#Remove duplicates
hoxGenes = tempHoxGenes %>% group_by(hgFixed.ggGeneName.gene) %>% filter(hg38.knownGene.txEnd - hg38.knownGene.txStart == max(hg38.knownGene.txEnd - hg38.knownGene.txStart))

#Histogram of lenght
qplot(abs(hoxGenes$hg38.knownGene.txEnd-hoxGenes$hg38.knownGene.txStart),geom = "histogram", bins = 10) + labs(x = "Lengths")

#Length of nearest cytozine in dinucleotide CG and sequence
x = c()
y = c()
counter = 0
for (i in 1:length(hoxGenes$hgFixed.ggGeneName.gene)){
  gc = gregexpr('CG', hoxGenes$hg38.knownGeneMrna.seq)
  for (i in 1:nchar(hoxGenes$hg38.knownGeneMrna.seq)) {
    min = min(abs(gc[[1]][1:length(gc[[1]])] - i))
    y[counter] <- min
    x[counter] <- i
    counter = counter + 1
  }
}
dists = data.frame(x, y)
ggplot(data = dists, aes(x = x, y = y)) + geom_bar(colour = "black", stat = "identity") + labs(x = "Position") + labs(y = "Lenght")
  
#Visualize of GRanges HOX genes
counter = 0
starts = c()
ends = c()
for (i in 1:length(hoxGenes$hgFixed.ggGeneName.gene)){
  startExons = strsplit(hoxGenes$hg38.knownGene.exonStarts[i], ',')[[1]]
  endExons = strsplit(hoxGenes$hg38.knownGene.exonEnds[i], ',')[[1]]
  for (j in 1:length(startExons)){
    starts[counter] = as.numeric(startExons[j])
    ends[counter] = as.numeric(endExons[j]) 
    counter = counter + 1
  }
}
ir = IRanges(start = starts, end = ends)
gr = GRanges(RangedData(ir))
atrack <- AnnotationTrack(gr, name = "HOX")
plotTracks(atrack)
