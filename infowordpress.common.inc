<?php
// $Id$
/**
 * @file
 * Common functions: to get results from ws, to encrypt and decrypt passwords.
 *
 */
/**
 * Request the web service to obtain results. Perform an array results, based on limit, to get only the
 * results required to render in a block. 
 *
 * This function allows to page results obtained with the web service. To show pager controls its necessary
 * to use the 'infoutilities_pager' theme .
 *
 * @param $limit
 *   The number of ws results to display per page.
 * @param $element
 *   id of block to distinguish between multiple pagers on one page 
 * @param $parameters
 *   array containing the web service arguments
 * @return
 *   array containing web service results, or FALSE if there was an error
 *
 */
function infowordpress_ws_result($ws_url, $limit = 15, $element, $parameters = array()) {

  if (!$fp = @fopen($ws_url, "r")) {
    if (user_is_logged_in()) {//show error only inside administration
      drupal_set_message(t('Sorry, this wordpress service is not available now. Try later. Url:').$ws_url, 'error', FALSE);
    }
	return FALSE;
  }

  $from = 0;
  if ($parameters['type'] == 3 OR ($parameters['type'] == 2 && ($parameters['display'] == 0 OR $parameters['display']==1))) {
    $from = infoutilities_pager_get_from( $limit, $element);
  }

  //search with XMLRPC ws
  switch ($parameters['type']) {
    case 1: //infopost
      //get a post by id
      $att = isset($parameters['attachments']) ? $parameters['attachments'] : 0;
	  $result = xmlrpc($ws_url, 'info.getPost', $limit, $att);
      $tipo = t("Infopost");
	  break;

    case 2: //inforecents
	  $date = unserialize($parameters['date']);
	  //print_r($date);
      $att = isset($parameters['attachments']) ? $parameters['attachments'] : 0;
	  switch ($parameters['display']) {
        case 1: //get a list of full recent posts
        case 4: 
	      if(!strlen($parameters['category']) AND !strlen($parameters['author'])  AND
		     !$date['year'] AND !$date['month'] AND !$date['day']) {
		    $result = xmlrpc($ws_url, 'info.getRecentPosts', $from, $limit, $att);  
	      }
		  else {
		    $limit = $limit ? $limit : -1; //to get all
		    $result = xmlrpc($ws_url, 'info.advancedSearch', NULL, $parameters['category'], $date['year'], $date['month'], $date['day'], $parameters['author'], $from, $limit, 1, $att);  
	      }
		  break;

	    default: //get a list of recent posts titles
	      if(!strlen($parameters['category']) AND !strlen($parameters['author'])  AND
		     !$date['year'] AND !$date['month'] AND !$date['day']) {
	        $result = xmlrpc($ws_url, 'info.getRecentPostTitles', $from, $limit, $att);  
	      }
		  else {
		    $limit = $limit ? $limit : -1; //to get all
			$result = xmlrpc($ws_url, 'info.advancedSearch', NULL, $parameters['category'], $date['year'], $date['month'], $date['day'], $parameters['author'], $from, $limit, 0, $att);  
	      }
	      break;
	  }
      $tipo = t("Inforecents");
      break;

    case 3: //infoblogs
      //get blogs and users
      $result = xmlrpc($ws_url, 'info.getBlogs', (int)$from, $limit, $parameters['login'], $parameters['pass'], $parameters['display']);
      //$result = xmlrpc($ws_url, 'info.getUsers_byBlogId', 1, $parameters['login'], $parameters['pass'], $parameters['display']);
      $tipo = t("Infoblog");
      break;

    case 4: //infocategories
      //get blog's categories
      $result = xmlrpc($ws_url, 'info.getCategories');
      $tipo = t("Infocategories");
      break;
  }
  if (!$result) {
    if (xmlrpc_error_msg() && user_is_logged_in()) {//show error only inside administration
      list(, $id) = explode('-', $element);
      $link_editblock = l(t('Edit block'), 'admin/infowordpress/block/edit/'. $parameters['type'] .'/'. $id);
	  drupal_set_message(t("Infowordpress Error") .':'. xmlrpc_error_msg() .', '. t('on block ') . $element .'. '. $link_editblock);
	}
    return FALSE;
  }
  if ($parameters['type'] == 3 OR ($parameters['type'] == 2 && ($parameters['display'] == 0 OR $parameters['display']==1))) {
    //set at pager total of results and total of pages
    infoutilities_pager_set_totals($limit, $element, $result['total']);
  }

  return $result;
}
function infowordpress_encryptData($value, $key) {
  
  $text = $value;
  $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
  $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
  $crypttext = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $text, MCRYPT_MODE_ECB, $iv));
  
  return $crypttext;
}

function infowordpress_decryptData($value, $key) {
  
  $crypttext = $value;
  $iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
  $iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
  $decrypttext = trim(mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($crypttext), MCRYPT_MODE_ECB, $iv));
  
  return $decrypttext;
}

