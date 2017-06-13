<xsl:stylesheet
     xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
     xmlns:d = "http://docbook.org/ns/docbook"
     exclude-result-prefixes="d"
     version = '1.0'>

    <xsl:param name="shade.verbatim" select="1"/>
    <xsl:attribute-set name="shade.verbatim.style">
      <xsl:attribute name="background-color">#E0E0E0</xsl:attribute>
      <xsl:attribute name="border-width">0.5pt</xsl:attribute>
      <xsl:attribute name="border-style">solid</xsl:attribute>
      <xsl:attribute name="border-color">#575757</xsl:attribute>
      <xsl:attribute name="padding">3pt</xsl:attribute>
    </xsl:attribute-set>

</xsl:stylesheet>
