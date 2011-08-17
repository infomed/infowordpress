
infowordpress module 1.1
Originally By: Yazna Garcia (yazna@infomed.sld.cu)
Maintained By: Yazna Garcia (yazna@infomed.sld.cu)

April 07, 2008

DESCRIPTION
  * allows to view published posts and blogs from Wordpress Mu.
  * provides integration with wordpress's XMLRPC web services.
  * It requires infoxmlrpc plugin (https://github.com/infomed/infoxmlrpc) for Wp Mu, at WpMu instance!.
  * allows 4 kind of blocks:
    * infopost: to show a wordpress post, 
    * inforecents: to show list of recents post,
    * infoblogs: to show list of blogs and their administrators at a wp mu instance. 
    * infocategories: to show list of blog's categories and posts associated to each category

INSTRUCTIONS:

1) Be sure that the plugin infoxmlrpc is actived in the instances of WpMu from where you want to recover the information

2) copy this to your modules directory

3) enable it at administer->build->modules

4) assign module's permissions at administer->infowordpress->permissions

5) configure it at administer->infowordpress->settings (url of WpMu instances, User an Password only for Infoblogs)

6) configure infowordpress blocks at administer->infowordpress->blocks

7) use infopost block to show one published post from wordpress

8) use inforecents block to show list of published recents post from a wordpress blog

9) use infoblogs block to show list of blogs and their administrators at a WpMu instance

10) use infocategories block to show list of categories from a wordpress blog, where each category have a link to show posts for the category.

11) assign infowordpress blocks to a region at administer->build->blocks

12) disable module at administer->modules

13) delete all module's data at administer->modules->uninstall

NOTE: For security reasons active mode storing encrypted password for which:

1) Its neccesary to activate mcrypt support in PHP (extension=php_mcrypt.dll); and  be sure that Php5 
installation have libmcrypt.dll .  


