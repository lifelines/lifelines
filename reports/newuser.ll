/*
 * @progname       newuser
 * @version        1.0 (2005/10/19)
 * @author         Lawrence M. Hamilton, Jr.
 
 * @category       test

 * @output         Text

 * @description    Sample report for a new user.

Sample report to print out basic information about the current database
and version of Lifelines, plus the user properties. Designed with new 
LifeLines users in mind.

This is useful as a simple example of what can be done with LifeLines' 
Report Language, and as a building block for new report authors. It 
also is good for testing a new LifeLines installation to verify the 
values in the LifeLines configuration file.

See the ll-reportmanual in the Documentation Folder and review the 
other reports in the Programs directory for examples and ideas.

*/

proc main()
{
	"Database and Version are controlled by the LifeLines program" nl()
	"and are not dependent on the configuration file." nl()nl()

	"If one of the other lines is blank, then that value" nl()
	"is not set in your LifeLines' configuration file," nl()
	"lines.cfg on Windows and .linesrc on *nix." nl()nl()

	"Database: " database() nl()
	"Version: " version() nl()
	"Name: " getproperty("user.fullname") nl()
	"Address: " getproperty("user.address") nl()
	"Phone: " getproperty("user.phone") nl()
	"Email: " getproperty("user.email") nl()
	"Web: " getproperty("user.url") nl()nl()

	"The values to check in the configuration file are:" nl()nl()
	"user.fullname=" nl()
	"user.address=" nl()
	"user.phone=" nl()
	"user.email=" nl()
	"user.phone=" 
}
