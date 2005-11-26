/* 
   Copyright (c) 2001-2005 Perry Rapp
   "The X11 license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ISOLANGS_H_INCLUDED
#define ISOLANGS_H_INCLUDED



static const char * langs[] = {
	"ar", "Arabic"
	, "az", "Azerbaijani"
	, "be", "Belarusian"
	, "bg", "Bulgarian"
	, "bn", "Bengali"
	, "bo", "Tibetan"
	, "ca", "Catalan"
	, "ce", "Chechen"
	, "cs", "Czech"
	, "cu", "Slavonic"
	, "cy", "Welsh"
	, "da", "Danish"
	, "de", "German"
	, "el", "Greek"
	, "en", "English"
	, "eo", "Esperanto"
	, "es", "Spanish"
	, "et", "Estonian"
	, "eu", "Basque"
	, "fa", "Persian"
	, "fi", "Finnish"
	, "fr", "French"
	, "ga", "Irish"
	, "gd", "Gaelic"
	, "gl", "Gallegan"
	, "gu", "Gujarati"
	, "gv", "Manx"
	, "he", "Hebrew"
	, "hi", "Hindi"
	, "hr", "Croation"
	, "hu", "Hungarian"
	, "hy", "Armenian"
	, "id", "Indonesian"
	, "is", "Icelandic"
	, "it", "Italian"
	, "iw", "Hebrew"
	, "ja", "Japanese"
	, "jv", "Javanese"
	, "ka", "Georgian"
	, "kk", "Kazakhr"
	, "km", "Khmer"
	, "kn", "Kannada"
	, "ko", "Korean"
	, "la", "Latin"
	, "lo", "Lao"
	, "mi", "Maori"
	, "mk", "Macedonian"
	, "ml", "Malayalam"
	, "mn", "Mongolian"
	, "ms", "Malay"
	, "mt", "Maltese"
	, "my", "Burmese"
	, "nl", "Dutch"
	, "no", "Norwegian"
	, "pl", "Polish"
	, "pt", "Portuguese"
	, "ro", "Romanian"
	, "ru", "Russian"
	/*, "sh", "Serbian Latin" */
	, "sl", "Slovene"
	, "sk", "Slovak"
	/* , "sr", "Serbian Cyrillic" */
	/*, "sh", "Serbian Latin" */
	, "sv", "Swedish"
	, "ta", "Tamil"
	, "te", "Telugu"
	, "tr", "Turkish"
	, "za", "Zhuang"
	, "zh", "Chinese"
	, "zu", "Zulu"
	, 0, 0
};
/*
 http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html 
*/
static const char * countries[] = {
	/* TODO: Obviously this isn't even close to complete */
	"AD", "Andorra"
	, "AF", "Afghanistan"
	, "AG", "Antigua and Barbuda"
	, "AI", "Anguilla"
	, "AL", "Albania"
	, "AM", "Armenia"
	, "AO", "Angola"
	, "AQ", "Antartica"
	, "AR", "Argentina"
	, "AS", "American Samoa"
	, "AT", "Austria"
	, "AU", "Australia"
	, "AW", "Aruba"
	, "AX", "Aland Islands"
	, "AZ", "Azerbaijan"
	, "BA", "Bosnia and Herzegovina"
	, "BB", "Barbados"
	, "BD", "Bangladesh"
	, "BE", "Belgium"
	, "BF", "Burkina Faso"
	, "BG", "Bulgaria"
	, "BH", "Bahrain"
	, "BI", "Burundi"
	, "BJ", "Benin"
	, "BM", "Bermuda"
	, "BN", "Brunei Darussalam"
	, "BO", "Bolivia"
	, "BR", "Brazil"
	, "BS", "Bahamas"
	, "BT", "Bhutan"
	, "BV", "Bouvet Island"
	, "BW", "Botswana"
	, "BY", "Belarus"
	, "BZ", "Belize"
	, "DZ", "Algeria"
	, "ES", "Spain"
	, "FI", "Finland"
	, "IO", "British Indian Ocean Territory"
	, "IT", "Italy"
	, "PL", "Poland"
	, "SE", "Sweden"
	, "US", "United States"
	, 0, 0
};


#endif /* ISOLANGS_H_INCLUDED */
