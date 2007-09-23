<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE xsl:stylesheet [<!ENTITY nbsp "&#160;">]>

<!--
  ++ Automated Testing Framework (atf)
  ++
  ++ Copyright (c) 2007 The NetBSD Foundation, Inc.
  ++ All rights reserved.
  ++
  ++ Redistribution and use in source and binary forms, with or without
  ++ modification, are permitted provided that the following conditions
  ++ are met:
  ++ 1. Redistributions of source code must retain the above copyright
  ++    notice, this list of conditions and the following disclaimer.
  ++ 2. Redistributions in binary form must reproduce the above copyright
  ++    notice, this list of conditions and the following disclaimer in the
  ++    documentation and/or other materials provided with the distribution.
  ++ 3. All advertising materials mentioning features or use of this
  ++    software must display the following acknowledgement:
  ++        This product includes software developed by the NetBSD
  ++        Foundation, Inc. and its contributors.
  ++ 4. Neither the name of The NetBSD Foundation nor the names of its
  ++    contributors may be used to endorse or promote products derived
  ++    from this software without specific prior written permission.
  ++
  ++ THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
  ++ CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
  ++ INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  ++ MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  ++ IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
  ++ DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  ++ DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  ++ GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  ++ INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  ++ IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  ++ OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  ++ IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  -->

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Parameters that can be overriden by the user. -->
  <xsl:param name="global.css">tests-results.css</xsl:param>
  <xsl:param name="global.title">ATF Tests Results</xsl:param>

  <xsl:template match="/">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()" />
    </xsl:copy>
  </xsl:template>

  <xsl:template match="tests-results">
    <html xmlns="http://www.w3.org/1999/xhtml">
      <head>
        <meta http-equiv="Content-Type"
              content="text/html; charset=iso-8859-1" />
        <link rel="stylesheet" type="text/css" href="{$global.css}" />

        <title><xsl:value-of select="$global.title" /></title>
      </head>

      <body>
        <h1><xsl:value-of select="$global.title" /></h1>

        <xsl:call-template name="results-summary" />
        <xsl:call-template name="tcs-summary" />

        <xsl:apply-templates mode="details" />
      </body>
    </html>
  </xsl:template>

  <xsl:template name="results-summary">
    <h2>Results summary</h2>

    <xsl:variable name="ntps" select="count(tp)" />
    <xsl:variable name="ntps-failed" select="count(tp/failed)" />
    <xsl:variable name="ntcs" select="count(tp/tc)" />
    <xsl:variable name="ntcs-passed" select="count(tp/tc/passed)" />
    <xsl:variable name="ntcs-failed" select="count(tp/tc/failed)" />
    <xsl:variable name="ntcs-skipped" select="count(tp/tc/skipped)" />

    <table class="results-summary">
      <tr>
        <th>Item</th>
        <th>Count</th>
      </tr>
      <tr class="group">
        <td><p>Test programs</p></td>
        <td class="numeric"><p><xsl:value-of select="$ntps" /></p></td>
      </tr>
      <tr class="entry">
        <td><p>Bogus test programs</p></td>
        <xsl:choose>
          <xsl:when test="$ntps-failed > 0">
            <td class="numeric-error">
              <p><xsl:value-of select="$ntps-failed" /></p>
            </td>
          </xsl:when>
          <xsl:otherwise>
            <td class="numeric">
              <p><xsl:value-of select="$ntps-failed" /></p>
            </td>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
      <tr class="group">
        <td><p>Test cases</p></td>
        <td class="numeric"><p><xsl:value-of select="$ntcs" /></p></td>
      </tr>
      <tr class="entry">
        <td><p>Passed test cases</p></td>
        <td class="numeric"><p><xsl:value-of select="$ntcs-passed" /></p></td>
      </tr>
      <tr class="entry">
        <td><p>Failed test cases</p></td>
        <xsl:choose>
          <xsl:when test="$ntcs-failed > 0">
            <td class="numeric-error">
              <p><xsl:value-of select="$ntcs-failed" /></p>
            </td>
          </xsl:when>
          <xsl:otherwise>
            <td class="numeric">
              <p><xsl:value-of select="$ntcs-failed" /></p>
            </td>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
      <tr class="entry">
        <td><p>Skipped test cases</p></td>
        <xsl:choose>
          <xsl:when test="$ntcs-skipped > 0">
            <td class="numeric-warning">
              <p><xsl:value-of select="$ntcs-skipped" /></p>
            </td>
          </xsl:when>
          <xsl:otherwise>
            <td class="numeric">
              <p><xsl:value-of select="$ntcs-skipped" /></p>
            </td>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
    </table>
  </xsl:template>

  <xsl:template name="tcs-summary">
    <h2>Test cases summary</h2>

    <table class="tcs-summary">
      <tr>
        <th><p>Test case</p></th>
        <th><p>Result</p></th>
        <th><p>Reason</p></th>
      </tr>
      <xsl:apply-templates mode="summary" />
    </table>
  </xsl:template>

  <xsl:template match="tp" mode="summary">
    <tr>
      <td class="tp-id" colspan="3">
        <p><xsl:value-of select="@id" /></p>
      </td>
    </tr>
    <p><xsl:apply-templates select="tc" mode="summary" /></p>
  </xsl:template>

  <xsl:template match="tc" mode="summary">
    <xsl:variable name="full-id"
                  select="concat(translate(../@id, '/', '_'), '_', @id)" />
    <tr>
      <td class="tc-id">
        <p><a href="#{$full-id}"><xsl:value-of select="@id" /></a></p>
      </td>
      <xsl:apply-templates select="passed|failed|skipped" mode="tc" />
    </tr>
  </xsl:template>

  <xsl:template match="passed" mode="tc">
    <td class="tcr-passed"><p>Passed</p></td>
    <td><p>N/A</p></td>
  </xsl:template>

  <xsl:template match="failed" mode="tc">
    <td class="tcr-failed"><p>Failed</p></td>
    <td><p><xsl:apply-templates /></p></td>
  </xsl:template>

  <xsl:template match="skipped" mode="tc">
    <td class="tcr-skipped"><p>Skipped</p></td>
    <td><p><xsl:apply-templates /></p></td>
  </xsl:template>

  <xsl:template match="tp" mode="details">
    <xsl:apply-templates select="tc" mode="details" />
  </xsl:template>

  <xsl:template match="tc" mode="details">
    <xsl:variable name="full-id"
                  select="concat(translate(../@id, '/', '_'), '_', @id)" />

    <a name="{$full-id}" />
    <h2 id="{$full-id}">Test case:
    <xsl:value-of select="../@id" /><xsl:text>/</xsl:text>
    <xsl:value-of select="@id" /></h2>

    <h3>Standard output stream</h3>
    <p class="so"><xsl:apply-templates select="so" mode="details" /></p>

    <h3>Standard error stream</h3>
    <p class="se"><xsl:apply-templates select="se" mode="details" /></p>
  </xsl:template>

  <xsl:template match="so" mode="details">
    <xsl:apply-templates /><br />
  </xsl:template>

  <xsl:template match="se" mode="details">
    <xsl:apply-templates /><br />
  </xsl:template>

  <xsl:template match="@*|node()" priority="-1">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()" />
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
