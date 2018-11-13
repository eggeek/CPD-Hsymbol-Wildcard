master.pdf: *.tex
	-pdflatex master
	-bibtex master
	-pdflatex master
	-pdflatex master
clean:
	rm -f *.aux *.bbl *.blg *.log master.pdf
