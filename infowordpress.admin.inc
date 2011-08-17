<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the infowordpress module.
 */
include_once drupal_get_path('module', 'infowordpress') .'/infowordpress.common.inc';

/**
 * Administration page of infowordpress, shows a list of links to module options
 */
function infowordpress() {
  //igual que system_admin_menu_block_page
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  $output = theme('admin_block_content', $content);
  return $output;
}

/**
 * Administration page of infowordpress blocks
 */
function infowordpress_admin() {

  $block_types = array (
    1 => t("Infopost"),
	2 => t("Inforecents"),
    3 => t("Infoblog"),
    4 => t("Infocategories"),
  );

  $header = array(t("Title"), t("Type"), t("Region"), t("Operations"));
  $result = pager_query("SELECT iwpid, title, type FROM {infowordpress_block} ORDER BY type", 30);
  
  $def_theme = variable_get('theme_default', 'garland');
  while ($u = db_fetch_object($result)) {
    $region = db_result(db_query("SELECT region FROM {blocks} WHERE delta = '%s' AND theme = '%s'", 'infowordpress-'.$u->iwpid, $def_theme));
    $edit = l(t('edit'), "admin/infowordpress/block/edit/$u->type/$u->iwpid");
	$delete = l(t('delete'), "admin/infowordpress/block/delete/$u->iwpid");
	//$destination para que retorne a blocks del modulo
	$destination = "destination=admin/infowordpress/block";
	$configure = l(t('configure'), "admin/build/block/configure/infowordpress/infowordpress-$u->iwpid", array("query" => $destination));

	$rows[] = array(
      array("data" => check_plain($u->title)),
      array("data" => $block_types[$u->type], "align" => "center"),
      array("data" => $region, "align" => "center"),
      array("data" => $edit .' | '. $delete .' | '. $configure)
    );
  }
  if (!count($rows)) {
    $rows[] = array(array("data" => t("No infowordpress blocks exist."), "colspan" => "4"));
  }
  $pager = theme("pager", NULL, 30);
  if (!empty($pager)) {
    $rows[] = array(array("data" => $pager, "colspan" => "4"));
  }

  $output = theme('table', $header, $rows);
  return $output;
}

/**
 * Block's Edit form
 *
 * @param $type
 *   type of infowordpress block (post=1, recents=2) to edit
 * @param $iwpid
 *   id of infowordpress block to edit
 * @return
 *   associative array $form of $iwpid infowordpress block
 *
 * @ingroup forms
 * @see infowordpress_block_edit_submit()
 * @see infowordpress_block_form_validate()
 */
function infowordpress_block_form(&$form_state, $type, $iwpid = 0) {

  $iwpid = isset($form_state['storage']['stored_iwpid']) ? $form_state['storage']['stored_iwpid'] : $iwpid;

  $block = $iwpid ? infowordpress_load_block($iwpid) : array();

  if (isset($form_state['storage'])) {
    $type = $form_state['storage']['stored_type'];
    $title = $form_state['storage']['stored_title'];
    $descr = $form_state['storage']['stored_descr'];
    $descr_place = $form_state['storage']['stored_descr_place'];
    $setid = $form_state['storage']['stored_setid'];
    $blogname = $form_state['storage']['stored_blogname'];
    $category = $form_state['storage']['stored_category'];
    $author = $form_state['storage']['stored_author'];
    $date = $form_state['storage']['stored_date'];
    $item_link = $form_state['storage']['stored_item_link'];
    $display = $form_state['storage']['stored_display'];
    $display_others = $form_state['storage']['stored_display_others'];
    $region = $form_state['storage']['stored_region'];
    $def_theme = $form_state['storage']['stored_def_theme'];
  }
  else {
    $title = $block['title'];
    $descr = $block['descr'];
    $descr_place = $block['descr_place'];
    $setid = $block['setid'];
    $blogname = $block['blogname'];
    $category = $block['category'];
    $author = $block['author'];
    $date = $block['date'] ? unserialize($block['date']) : NULL;
    $item_link = $block['item_link'] ? $block['item_link'] : 0;
    $display = isset($block['display']) ? $block['display'] : 1;
    $display_others = isset($block['display_others']) ? explode('_', $block['display_others']) : array(0);
    
    $def_theme = variable_get('theme_default', 'garland');
	$b_region = '';
	if ($iwpid) {
	  $b_region = db_result(db_query("SELECT region FROM {blocks} WHERE delta = '%s' AND theme = '%s'", 'infowordpress-'.$iwpid, $def_theme));
	}
	$region = $b_region;
  }

  $form['iwpid'] = array(
    '#type' => 'hidden',
    '#value' => $iwpid,
  );
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $title,
    '#maxlength' => 64,
    '#description' => t('Enter the title of the block'),
    '#required' => TRUE,
  );
  $options = infowordpress_urls_list();
  
  switch ($type) {
    case 1: //infopost
      $form['filter'] = array(
        '#type' => 'fieldset',
        '#title' => t('Set Blog, Filter options and Refresh.'),
      );
      $form['filter']['setid'] = array(
        '#type' => 'select',
        '#title' => t('Url of Wordpress Mu'),
        '#default_value' => $setid,
        '#options' => $options,
        '#description' => t('Select the Url of Wordpress Mu.'),
        '#required' => TRUE,
      );
      $form['filter']['blogname'] = array(
        '#type' => 'textfield',
        '#title' => t('Blog Name'),
        '#default_value' => $blogname,
        '#maxlength' => 255,
        '#description' => t('The name of blog at Wordpress Mu. Leave blank if is the root blog.') . t(' WARNING! If you change it, you have to Refresh Posts.'),
        '#required' => FALSE,
      );
      $form['filter']['category'] = array(
        '#type' => 'textfield',
        '#title' => t('Category'),
        '#default_value' => $category,
        '#maxlength' => 255,
        '#description' => t('The name of category to filter posts.'),
        '#required' => FALSE,
      );
      $form['filter']['author'] = array(
        '#type' => 'textfield',
        '#title' => t('Author'),
        '#default_value' => $author,
        '#maxlength' => 255,
        '#description' => t('The author of posts, to filter by him.'),
        '#required' => FALSE,
      );
      $form['filter']['date'] = array(
        '#type' => 'infoutilities_date_null_noval', //definido en infoutilities
        '#title' => t('Date'),
        '#default_value' => $date,
        '#description' => t("You can filter by year or month-year or day-month. Leave blank the part of the date for which doesn't want to filter"),
        '#required' => FALSE,
      );
      //mostrar boton de refresh Posts
      $form['filter']['refresh'] = array(
        '#type' => 'submit',
        '#value' => t('Refresh Posts'),
        '#submit' => array('infowordpress_block_refresh_submit'),
      );

      if (isset($form_state['storage']['list_options'])) {
        //after submit Posts by Title, list of post is seted but no one selected
        $options_post = $form_state['storage']['list_options'];
		$default = NULL;
      }
      else {
        if (isset($block['numentry'])) {
          //after edit post, list of post is seted and selected post = numentry
          $default = $block['numentry'];
          $options_post = infowordpress_get_posts_titles($block['setid'], $blogname, $category, $author, $date);
        }
      }
      if ($options_post) {
        $form['numentry'] = array(
          '#type' => 'radios',
          '#title' => t('Select a Post'),
          '#options' => $options_post,
          '#default_value' => $default,
          //'#required' => TRUE, //para el refresh, en submit validar que no este vacio
          '#description' => t('Select the Title of Post that you want to show.'),
        );
      }
	  else {
	    if ($iwpid) {
	      form_set_error('blogname', t('Sorry, there are no Posts at this Blog name, with those filter options.'));
		}
      }
	  
      $form['show'] = array(
        '#type' => 'fieldset',
        '#title' => t('View in block'),
      );
      $form['show']['display'] = array(
        '#type' => 'radios',
        '#title' => t('Display'),
        '#options' => array(1 => t('Whole post'), 2 => t('Teaser')),
        '#default_value' => $display,
        '#description' => t('What part of post you want to show in the block.'),
      );
      $form['show']['display_others'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Display also'),
        '#options' => array( 1 => t('Date'), 2 => t('Author'), 3 => t('Categories'), 4 => t('Attachments')),
        '#default_value' => $display_others,
      );
	  break;
  
    case 2: //inforecent
  	  $form['descr'] = array(
        '#type' => 'textarea',
        '#title' => t('Description'),
        '#default_value' => $descr,
        '#description' => t('Enter the description of the block'),
      );
      $form['descr_place'] = array(
        '#type' => 'select',
        '#title' => t('Show Description'),
        '#default_value' => $descr_place ? $descr_place : 0,
        '#options' => array(0 => '', 1 => t('Up of the Items'), 2 => t('Below of the Items')),
        '#description' => t('Select the place where you want to show Description.'),
      );
      $form['setid'] = array(
        '#type' => 'select',
        '#title' => t('Url of Wordpress Mu'),
        '#default_value' => $setid,
        '#options' => $options,
        '#description' => t('Select the Url of Wordpress Mu.'),
        '#required' => TRUE,
      );
      $form['blogname'] = array(
        '#type' => 'textfield',
        '#title' => t('Blog Name'),
        '#default_value' => $blogname,
        '#maxlength' => 255,
        '#description' => t('The name of blog at Wordpress Mu. Leave blank if is the root blog.'),
        '#required' => FALSE,
      );
      
	  $form['filter'] = array(
        '#type' => 'fieldset',
        '#title' => t('Filter by:'),
      );
      $form['filter']['category'] = array(
        '#type' => 'textfield',
        '#title' => t('Category'),
        '#default_value' => $category,
        '#maxlength' => 255,
        '#description' => t('The name of category to filter posts.'),
        '#required' => FALSE,
      );
      $form['filter']['author'] = array(
        '#type' => 'textfield',
        '#title' => t('Author'),
        '#default_value' => $author,
        '#maxlength' => 255,
        '#description' => t('The author of posts, to filter by him.'),
        '#required' => FALSE,
      );
      $form['filter']['date'] = array(
        '#type' => 'infoutilities_date_null_noval', //definido en infoutilities
        '#title' => t('Date'),
        '#default_value' => $date,
        '#description' => t("You can filter by year or month-year or day-month. Leave blank the part of the date for which doesn't want to filter"),
        '#required' => FALSE,
      );
	  
      $form['show'] = array(
        '#type' => 'fieldset',
        '#title' => t('View in block'),
      );
      $form['show']['numentry'] = array(
        '#type' => 'textfield',
        '#title' => t('Max Posts'),
        '#default_value' => $block['numentry'] ? $block['numentry'] : 10,
        '#size' => 2,
        '#maxlength' => 2,
        '#description' => t('The maximum number of %content that will appear in the block.', array('%content' => t('posts'))),
        '#required' => TRUE,
      );
      $form['show']['display'] = array(
        '#type' => 'radios',
        '#title' => t('Display Posts'),
        '#options' => array(1 => t('Results Page'), 2 => t('Items List, with more... link'), 3 => t('Items List'), 4 => t('Content List')),
        '#default_value' => $display,
        '#description' => t('How to show posts in the block.'),
      );
      $form['show']['item_link'] = array(
        '#type' => 'radios',
        '#title' => t('Items link'),
        '#options' => array( 0 => t('To Post'), 1 => t('To first Attachment')),
        '#default_value' => $item_link,
        '#description' => t('Select To Post if you want to get full post when you click an item. Select To first Attachment if you want to get the first post Attachment when you click an item.'),
      );
      /*$form['show']['display_others'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Display also'),
        '#options' => array( 1 => t('Date'), 2 => t('Author'), 3 => t('Categories')),
        '#default_value' => $display_others,
      );*/
      break;

    case 3: //infoblog
      $form['setid'] = array(
        '#type' => 'select',
        '#title' => t('Url of Wordpress Mu'),
        '#default_value' => $setid,
        '#options' => $options,
        '#description' => t('Select the Url of Wordpress Mu.'),
        '#required' => TRUE,
      );
      $form['show'] = array(
        '#type' => 'fieldset',
        '#title' => t('View in block'),
      );
      $form['show']['numentry'] = array(
        '#type' => 'textfield',
        '#title' => t('Max Blogs'),
        '#default_value' => $block['numentry'] ? $block['numentry'] : 10,
        '#size' => 2,
        '#maxlength' => 2,
        '#description' => t('The maximum number of %content that will appear in the block.', array('%content' => t('blogs'))),
        '#required' => TRUE,
      );
      $form['show']['postcount'] = array(
        '#type' => 'select',
        '#title' => t('Number of post for each blog'),
        '#options' => array('0' => t('No'), '1' => t('Yes')),        
		'#default_value' => $block['display_others'] ? $block['display_others'] : '1',
        '#description' => t('Select if you want to display the number of posts for each blog or not.'),
        '#required' => TRUE,
      );
      $form['show']['display'] = array(
        '#type' => 'select',
        '#title' => t('Users by Role'),
        '#options' => array(0 => t('None of them'), 1 => t('Administrator'), 2 => t('Editor'), 3 => t('Author'), 4 => t('Contributor'), 5 => t('Subscriber')),
        '#default_value' => $block['display'] ? $block['display'] : 0,
        '#description' => t('Select if you want to display blog users or not.'),
      );
      break;

    case 4: //infocategories
  	  $form['descr'] = array(
        '#type' => 'textarea',
        '#title' => t('Description'),
        '#default_value' => $descr,
        '#description' => t('Enter the description of the block'),
      );
      $form['descr_place'] = array(
        '#type' => 'select',
        '#title' => t('Show Description'),
        '#default_value' => $descr_place ? $descr_place : 0,
        '#options' => array(0 => '', 1 => t('Up of the Items'), 2 => t('Below of the Items')),
        '#description' => t('Select the place where you want to show Description.'),
      );
      $form['setid'] = array(
        '#type' => 'select',
        '#title' => t('Url of Wordpress Mu'),
        '#default_value' => $setid,
        '#options' => $options,
        '#description' => t('Select the Url of Wordpress Mu.'),
        '#required' => TRUE,
      );
      $form['blogname'] = array(
        '#type' => 'textfield',
        '#title' => t('Blog Name'),
        '#default_value' => $blogname,
        '#maxlength' => 255,
        '#description' => t('The name of blog at Wordpress Mu. Leave blank if is the root blog.'),
        '#required' => FALSE,
      );
	  
      //mostrar boton de refresh Categories
      $form['refresh'] = array(
        '#type' => 'submit',
        '#value' => t('Refresh Categories'),
        '#submit' => array('infowordpress_block_refresh_submit'),
      );

      if (isset($form_state['storage']['list_options'])) {
        //after submit refresh categ, list is seted but no one selected
        $options_cat = $form_state['storage']['list_options'];
		$default = NULL;
      }
      else {
        if ($iwpid) {
          list($options_cat, $default) = infowordpress_get_drupalcateg($iwpid);
        }
      }
     if ($options_cat) {
        $form['categories'] = array(
          '#type' => 'select',
          '#title' => t('Select Categories'),
          '#options' => $options_cat,
          '#default_value' => $default,
          //'#required' => TRUE, //para el refresh, en submit validar que no este vacio
          '#description' => t('Select the Categories that you want to show in block.'),
	      '#multiple' => TRUE,
          '#size' => count($options_cat),
        );
        $form['options_cat'] = array(//to save categories on drupal db
          '#type' => 'hidden',
          '#value' => serialize($options_cat),
        );
      }
	  else {
	    if ($iwpid) {
	      form_set_error('blogname', t('Sorry, there are no Categories at this Blog name.'));
		}
      }
      
	  $form['show'] = array(
        '#type' => 'fieldset',
        '#title' => t('List of Posts associated to the Category: '),
      );
      $numentry = isset($form_state['storage']) ? $form_state['storage']['stored_numentry'] : ($block['numentry'] ? $block['numentry'] : 10);
	  $form['show']['numentry'] = array(
        '#type' => 'textfield',
        '#title' => t('Max Posts'),
        '#default_value' => $numentry,
        '#size' => 2,
        '#maxlength' => 2,
        '#description' => t('The maximum number of %content that will appear in the block.', array('%content' => t('posts'))),
        '#required' => TRUE,
      );
      $form['show']['item_link'] = array(
        '#type' => 'radios',
        '#title' => t('Each Post have link'),
        '#options' => array( 0 => t('To Post'), 1 => t('To first Attachment')),
        '#default_value' => $item_link,
        '#description' => t('Select To Post if you want to get full post when you click an item. Select To first Attachment if you want to get the first post Attachment when you click an item.'),
      );
      break;
  }
  
  $form['def_theme'] = array(
    '#type' => 'hidden',
    '#value' => $def_theme,
  );
  //en que region ubicar el bloque
  $block_regions = infoutilities_regions() + array(BLOCK_REGION_NONE => '<'. t('none') .'>');
  
  $form['region'] = array(
    '#type' => 'select',
    '#title' => t('Region'),
    '#description' => t('Assign a block to a region.'),
    '#default_value' => $region ? $region : BLOCK_REGION_NONE,
    '#options' => $block_regions,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('infowordpress_block_form_submit'),
  );

  return $form;
}

/**
 * Validate the user input in infowordpress_block_form
 */
function infowordpress_block_form_validate($form, &$form_state) {

  $settings = variable_get('infowordpress_settings', NULL);
  $setid = $form_state['values']['setid'];
  $ws_url = $settings[$setid][0] . (($block->blogname == '') ? 'xmlrpc.php' : $block->blogname .'/xmlrpc.php');
  $ws_url = check_url($ws_url);
  if (!valid_url($ws_url)) {
    form_set_error('url', t('Sorry, that is not a valid URL.'));
  }

  $type = $form_state['values']['type'];
  switch ($type) {
    case 3:
      if ( $settings[$setid][1] == '' or $settings[$setid][2] == '' ) {
        form_set_error('login', t('Sorry, this block requieres login and password at settings.'));
      }
	  break;
  }

  //si al hacer Submit no se seteo numentry es pq en el blogname no hay posts, no se puede crear un bloque con el
  if ($form_state['clicked_button']['#value'] == t('Submit') && ($type == 1 && !$form_state['values']['numentry'])) {
    form_set_error('numentry', t('You have to select a post.'));
  }

  if ($type == 2 or ($type == 1 && !isset($form_state['values']['refresh']))) {
    if (!is_numeric($form_state['values']['numentry'])) {
      $msg = $type==1 ? t('You must insert a number of Post Id.') : t('You must insert a number for the maximum number of posts.');
      form_set_error('numentry', $msg);
    }
    else if ($form_state['values']['numentry'] <= 0) {
      $msg = $type==1 ? t('Post Id must be positive.') : t('Maximum number of posts must be positive.');
      form_set_error('numentry', $msg);
    }
  }
}

/**
 * Save to DB infowordpress block and return to infowordpress administration page
 */
function infowordpress_block_form_submit($form, &$form_state) {

  $display_others = '';
  if ($form_state['values']['display_others']) {
    $display_others = implode('_', $form_state['values']['display_others']);
  }
  if ($form_state['values']['postcount']) {
    $display_others = $form_state['values']['postcount'];
  }

  $date = serialize($form_state['values']['date']);
  $status = $form_state['values']['region'] != BLOCK_REGION_NONE;
  $region = $status ? $form_state['values']['region'] : '';
  $def_theme = $form_state['values']['def_theme'];

  if ($form_state['values']['iwpid']) {
    db_query("UPDATE {infowordpress_block} SET title='%s', descr='%s', descr_place=%d, setid=%d, blogname='%s', category='%s', author='%s', date='%s', item_link=%d, type=%d, numentry=%d, display=%d, display_others='%s' WHERE iwpid=%d", $form_state['values']['title'], $form_state['values']['descr'], $form_state['values']['descr_place'], $form_state['values']['setid'], $form_state['values']['blogname'], $form_state['values']['category'], $form_state['values']['author'], $date, $form_state['values']['item_link'], $form_state['values']['type'], $form_state['values']['numentry'], $form_state['values']['display'], $display_others, $form_state['values']['iwpid'] );  
    $iwpid = $form_state['values']['iwpid'];
    //se actualiza title, region y status en blocks
    db_query("UPDATE {blocks} SET region='%s', status = %d WHERE delta = '%s' AND theme='%s'", $region, $status, 'infowordpress-'.$iwpid, $def_theme);
    //guardar en log de drupal
    watchdog('block', "Updated '%module' block: '%title'.", array('%module' => 'Infowordpress', '%title' => $form_state['values']['title']));
  }
  else {
    db_query("INSERT INTO {infowordpress_block} (title, descr, descr_place, setid, blogname, category, author, date, item_link, type, numentry, display, display_others) VALUES ('%s', '%s', %d, %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, '%s')", $form_state['values']['title'], $form_state['values']['descr'], $form_state['values']['descr_place'], $form_state['values']['setid'], $form_state['values']['blogname'], $form_state['values']['category'], $form_state['values']['author'], $date, $form_state['values']['item_link'], $form_state['values']['type'], $form_state['values']['numentry'], $form_state['values']['display'], $display_others);
    $iwpid = db_last_insert_id('infowordpress_block', 'iwpid');
    //inserta con title, region, status, theme en blocks
    db_query("INSERT INTO {blocks} (module, delta, theme, status, weight, region) VALUES ('%s', '%s', '%s', %d, %d, '%s')", 'infowordpress', 'infowordpress-'.$iwpid, $def_theme, $status, -10, $region);
    //guardar en log de drupal
    watchdog('block', "Added '%module' block: '%title'.", array('%module' => 'Infowordpress', '%title' => $form_state['values']['title']));
  }
  if($form_state['values']['type'] == 4) { 
    //drupal_set_message(print_r($form_state,TRUE));
	if ($form_state['values']['iwpid']) {
	  db_query("DELETE FROM {infowordpress_block_categ} WHERE iwpid=%d", $form_state['values']['iwpid']);
    }
    
	//update categories in infowordpress_block_categ
	$categories = unserialize($form_state['values']['options_cat']);
	foreach ($categories as $id => $cat){
      $select = 0;
	  if(in_array($id, $form_state['values']['categories'])){
	    $select = 1;
	  }
      db_query("INSERT INTO {infowordpress_block_categ} (iwpid, cat_ID, cat_name, selected) VALUES (%d, %d, '%s', %d)", $iwpid, $id, $cat, $select);
	}
  }
  unset($form_state['storage']); //for redirect
  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/block';

  return;
}

/**
 * Refresh posts at infowordpress_block_form
 */
function infowordpress_block_refresh_submit($form, &$form_state) {
  
  switch($form_state['values']['type']){
    case 1: //infopost
      //Conects to wp ws to obtain array of post titles with postid as key
      $rows = infowordpress_get_posts_titles($form_state['values']['setid'], $form_state['values']['blogname'], $form_state['values']['category'], $form_state['values']['author'], $form_state['values']['date']);
      break;
	case 4://infocategories
      //Conects to wp ws to obtain array of categories
      $rows = infowordpress_get_wpcateg($form_state['values']['setid'], $form_state['values']['blogname']);
	  break;
  }  
  $form_state['storage'] = array(
     'stored_iwpid' =>  $form_state['values']['iwpid'],
     'stored_type' =>  $form_state['values']['type'],
     'stored_title' =>  $form_state['values']['title'],
     'stored_descr' =>  $form_state['values']['descr'],
     'stored_descr_place' =>  $form_state['values']['descr_place'],
     'stored_setid' =>  $form_state['values']['setid'],
     'stored_blogname' =>  $form_state['values']['blogname'],
     'stored_category' =>  $form_state['values']['category'],
     'stored_author' =>  $form_state['values']['author'],
     'stored_date' =>  $form_state['values']['date'],
     'stored_item_link' =>  $form_state['values']['item_link'],
     'stored_display' =>  $form_state['values']['display'],
     'stored_display_others' =>  $form_state['values']['display_others'],
     'stored_region' =>  $form_state['values']['region'],
     'stored_def_theme' =>  $form_state['values']['def_theme'],
     'stored_numentry' =>  $form_state['values']['numentry'],
     'list_options' => $rows);

    return;
}

/**
 * Form to confirm deletion of an infowordpress block
 *
 * @param $iwpid
 *   id of block to delete
 *
 * @ingroup forms
 * @see infowordpress_confirm_delete_form_submit()
 */
function infowordpress_confirm_delete_form(&$form_state, $iwpid) {
  $form['block'] = array('#type' => 'value', '#value' => $iwpid);
  $result = db_fetch_object(db_query("SELECT title FROM {infowordpress_block} WHERE iwpid=%d", $iwpid));
  $form['title'] = array('#type' => 'value', '#value' => $result->title);

  return confirm_form(
    $form,
    t('Are you sure you want to delete %title?', array('%title' => $result->title)),
    $_GET['destination'] ? $_GET['destination'] : 'admin/infowordpress/block',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit deletion of an infowordpress block and return to infowordpress administration page
 */
function infowordpress_confirm_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    infowordpress_delete_block($form_state['values']['block']);
    //guardar en log de drupal
    watchdog('block', "Deleted '%module' block: '%title'.", array('%module' => 'Infowordpress', '%title' => $form_state['values']['title']));
  }
  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/block';
}

/**
 * Delete a block from db and return to infowordpress administration page
 *
 * @param $iwpid
 *   id of block to delete
 */
function infowordpress_delete_block($iwpid) {
  db_query("DELETE FROM {infowordpress_block} WHERE iwpid=%d", $iwpid);
  db_query("DELETE FROM {blocks} WHERE module = 'infowordpress' AND delta = 'infowordpress-%d'", $iwpid);
  drupal_set_message(t('The block has been removed.'));
  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/block';
}

/**
 * load block from db
 *
 * @param $iwpid
 *   id of block to load
 * @retun
 *   an associative array of block's data
 */
function infowordpress_load_block($iwpid) {
  $obj = db_fetch_array(db_query("SELECT * FROM {infowordpress_block} WHERE iwpid=%d", $iwpid));
  return $obj;
}
/**
 * @} End of "Block Administration".
 */

/**
 * @name module settings 
 * @{
 * setting forms to define URLs of WpMu Services, encrypt wp passwords.
 */

/**
 * Setting page of infowordpress
 */
function infowordpress_settings() {
  global $user;

  $header = array("", t("Operations"));

  $rows[] = array(array("data" => t("Wordpress Url"), "class" => "region", "colspan" => "2"));
  $results = variable_get('infowordpress_settings', NULL);
  if ($results) {
    foreach ($results as $setid => $u) {
      $rows[] = array(
        array("data" => check_url($u[0]) ),
        array("data" => l(t('edit'), "admin/infowordpress/settings/edit/$setid") .' | '. l(t('delete'), "admin/infowordpress/settings/delete/$setid"))
      );
    }
  }
  else {
    $rows[] = array(array("data" => t("No Urls exist."), "colspan" => "2"));
  }

  if (user_access('configure infowordpress encrypt')) {
    //option to encrypt passw
    $rows[] = array(array("data" => t("Encrypt Passwords"), "class" => "region", "colspan" => "2"));  

    if (!extension_loaded ('mcrypt')) {
      variable_set('infowordpress_encrypt_pass', 0);
      $rows[] = array(
        array("data" => t('No')),
        array("data" => t('To encrypt passwords you have to enable extension:php_mcrypt.dll, at php.ini. Be sure that Php installation have libmcrypt.dll')/*, "colspan" => "2"*/)
      );
    }
    else {
      $encrypt_pass = variable_get('infowordpress_encrypt_pass', 0) ? t('Yes') : t('No');
      $rows[] = array(
        array("data" => $encrypt_pass),
        array("data" => l(t('edit'), "admin/infowordpress/settings/encrypt"))
      );
    }
  }

  $output = theme('table', $header, $rows);

  return $output;
}

/**
 * Form to configure wordpress server url, user and passw
 *
 * @ingroup forms
 * @see infowordpress_settings_form_submit()
 * @see infowordpress_settings_form_validate()
 */
function infowordpress_settings_form(&$form_state, $setid=0) {

  $settings = variable_get('infowordpress_settings', NULL);

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => check_url($settings[$setid][0]),
    '#maxlength' => 255,
    '#description' => t('The fully-qualified URL of the Wordpress Mu server( Endpoint). Ej. http://blogs.sld.cu/'),
    '#required' => TRUE,
  );
  $form['login'] = array('#type' => 'textfield',
    '#title' => t('Username'),
    '#size' => 60,
    //'#required' => TRUE,
    '#default_value' => $settings[$setid][1],
    '#description' => t('Enter the username at that Wordpress Mu, if you want the InfoBlogs block.'),
  );

  $form['pass'] = array('#type' => 'password_confirm',
    '#description' => t('Enter the password that accompanies your username in both fields.'),
    '#size' => 64,
    //'#required' => TRUE,
  );

  $form['setid'] = array(
    '#type' => 'hidden',
    '#value' => $setid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}
/**
 * Validate the user input at infowordpress_settings_form
 */
function infowordpress_settings_form_validate($form, &$form_state) {

  $url = check_url($form_state['values']['url']);

  /*$parts = parse_url($url);
  if (isset($parts['path']) && $parts['path'] != '/') {
    $url = substr($url, 0, strlen($url) - strlen($parts['path']) + 1); //+1 para que se quede el /
    drupal_set_message(t('The url was changed, only the domain is necessary'));
  }*/


  //url finish with /
  $reverse = strrev($url);
  if ( $reverse[0] != '/') {
    $url .= '/';
  }
  
  $ws_url = $url .'xmlrpc.php';
  $res = xmlrpc($ws_url);
  if (!$res &&  xmlrpc_errno() == -7) {
    form_set_error('url', t('There was an error. Check the Url'));
 }

 $form_state['values']['url'] = $url;
 $settings = variable_get('infowordpress_settings', NULL);
 if ($settings) {
    foreach ($settings as $setid => $setting) {
     if (($setting[0] == $form_state['values']['url']) && $form_state['values']['setid'] != $setid ) {
       form_set_error('url', t('Sorry, that URL already exists at settings.'));
     }
   }
 }

  if ($form_state['values']['login'] && $form_state['values']['pass']) {
    $res = xmlrpc($ws_url, 'info.login_pass', $form_state['values']['login'], $form_state['values']['pass']);
    if (!$res &&  xmlrpc_error_msg() && xmlrpc_errno() == '403') {
      form_set_error('login', xmlrpc_error_msg());
    }
  }  
}

/**
 * Save server url to DB and redirect to settings page
 */
function infowordpress_settings_form_submit($form, &$form_state) {
  
  $pass = '';
  
  if ($form_state['values']['login'] && $form_state['values']['pass']) {
    if (variable_get('infowordpress_encrypt_pass', 0) == 1) {
      $pass = infowordpress_encryptData($form_state['values']['pass'], variable_get('infowordpress_encrypt_key', 'wpmukey'));
    }
    else {
      $pass = $form_state['values']['pass'];
    }
  }

  $settings = variable_get('infowordpress_settings', NULL);
  $setid = $form_state['values']['setid'];
  if ($setid) {  //update
    $settings[$setid] = array($form_state['values']['url'], $form_state['values']['login'], $pass);
  }
  else {  //insert new
    if (!$settings) { //the first
      $settings[1] = array($form_state['values']['url'], $form_state['values']['login'], $pass);
    }
    else {  
      $settings[] = array($form_state['values']['url'], $form_state['values']['login'], $pass);
    }
  }
  variable_set('infowordpress_settings', $settings);

  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/settings';
}

/**
 * Form to configure if encrypt wordpress passwords or not
 *
 * @ingroup forms
 * @see infowordpress_settings_encrypt_submit()
 * @see infowordpress_settings_encrypt_validate()
 */
function infowordpress_settings_encrypt(&$form_state) {

  $form['encrypt'] = array(
    '#type' => 'select',
    '#title' => t('Encrypt Passwords'),
    '#options' => array(0 => t('No'), 1 => t('Yes')),
    '#default_value' => variable_get('infowordpress_encrypt_pass', 0),
    '#description' => t('Select if you want to encrypt or not, paswords that you enter to connect with Wordpress Web Service.'),
    '#required' => TRUE,
  );

  $form['key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#size' => 60,
    '#default_value' => variable_get('infowordpress_encrypt_key', 'wpmukey'),
    '#description' => t('Enter the key to encrypt passwords.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}
/**
 * Validate the user input at infowordpress_settings_encrypt
 */
function infowordpress_settings_encrypt_validate($form, &$form_state) {
  if ($form_state['values']['encrypt']) {
    if (!extension_loaded ('mcrypt')) {
      form_set_error('encrypt', t('To encrypt passwords you have to enable extension:php_mcrypt.dll, at php.ini. Be sure that Php installation have libmcrypt.dll'));
    }
    elseif (!$form_state['values']['key']) {
      form_set_error('key', t('You have to enter a key.'));
    }
  }
  
}

/**
 * Save server encrypt and key as drupal variable and redirect to settings page
 */
function infowordpress_settings_encrypt_submit($form, &$form_state) {
  $change_encrypt=0;
  if ($form_state['values']['encrypt'] != variable_get('infowordpress_encrypt_pass', 0)) {
    variable_set('infowordpress_encrypt_pass', $form_state['values']['encrypt']);
    $change_encrypt=1;
  }
  if ($form_state['values']['key'] != variable_get('infowordpress_encrypt_key', 'wpmukey')) {
    $old_key = variable_get('infowordpress_encrypt_key', 'wpmukey');
    variable_set('infowordpress_encrypt_key', $form_state['values']['key']);
  }

  //cases to update passwords at settings
  if ( $change_encrypt ) {
    if ($form_state['values']['encrypt'] == 1) {
      //encrypt pass with key, because: change from not encrypt to encrypt
      $case_encr = 'encrypt_key';
    }
    else {
      if (isset($old_key)) {
        //decrypt pass with oldkey, because: change from encrypt to not encrypt and change key
        $case_encr = 'decrypt_oldkey';
      }
      else {
        //decrypt pass with key, because: change from encrypt to not encrypt and dont change key
        $case_encr = 'decrypt_key';
      }
    }
  }
  else {
    if (isset($old_key) && $form_state['values']['encrypt'] == 1) {
      //decrypt pass with oldkey and encrypt with key, because: change key
      $case_encr = 'decrypt_oldkey_encrypt_key';
    }
  }

  if ( isset($case_encr) ) {
    $new_settings = array();
    $settings = variable_get('infowordpress_settings', NULL);
    foreach ($settings as $setid => $setting) {
      if ($setting[2] != '') { //password exists
        switch ($case_encr) {
          case 'encrypt_key':
            $pass = infowordpress_encryptData($setting[2], $form_state['values']['key']);
            break;

          case 'decrypt_oldkey':
            $pass = infowordpress_decryptData($setting[2], $old_key);
            break;

          case 'decrypt_key':
            $pass = infowordpress_decryptData($setting[2], $form_state['values']['key']);
            break;

          case 'decrypt_oldkey_encrypt_key':
            $pass = infowordpress_decryptData($setting[2], $old_key);
            $pass = infowordpress_encryptData($pass, $form_state['values']['key']);
            break;
        }
      }
      //update pass
      $new_settings[$setid] = array($setting[0], $setting[1], $pass);
    }

	variable_set(infowordpress_settings, $new_settings);
  }
  
  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/settings';
}

/**
 * Form to confirm deletion of an infowordpress setting
 *
 * @param $iwpid
 *   id of block to delete
 *
 * @ingroup forms
 * @see infowordpress_setting_delete_form_submit()
 */
function infowordpress_setting_delete_form(&$form_state, $setid) {
  $form['setting'] = array('#type' => 'value', '#value' => $setid);
  $settings = variable_get('infowordpress_settings', NULL);

  // Construct the list items, whit blocks titles.
  $block = db_query("SELECT title FROM {infowordpress_block} WHERE setid=%d", $setid);
  while ($u = db_fetch_object($block)) {
    $blocks[] = check_plain($u->title);
  }

  if (isset($blocks)) {
    //there are blocks using that url
    $form['blocks'] = array('#value' => '<p>'. t('The following blocks are using this setting. They will be deleted, and <em>all data from them will be lost</em>!') .'</p>'. theme('item_list', $blocks));
    return confirm_form(
      $form,
      t('Confirm deletion'),
      'admin/infowordpress/settings',
      t('Are you sure you want to delete %url and blocks above?. <p>This action cannot be undone.</p>', array('%url' => check_url($settings[$setid][0]))),
      t('Delete'),
      t('Cancel')
    );
  }
  else {
    return confirm_form(
      $form,
      t('Are you sure you want to delete %url?', array('%url' => check_url($settings[$setid][0]))),
      $_GET['destination'] ? $_GET['destination'] : 'admin/infowordpress/settings',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
    );
  }
}

/**
 * Submit deletion of an infowordpress block and return to infowordpress administration page
 */
function infowordpress_setting_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $setid = $form_state['values']['setting'];
	$settings = variable_get('infowordpress_settings', NULL);
	unset($settings[$setid]);
	variable_set('infowordpress_settings', $settings);

    db_query("DELETE FROM {infowordpress_block} WHERE setid=%d", $setid);
  }
  cache_clear_all();
  $form_state['redirect'] = 'admin/infowordpress/settings';
}

function infowordpress_urls_list() {

  $settings = variable_get('infowordpress_settings', NULL);
  $options = array();
  foreach ($settings as $setid => $setting) {
    $options[$setid] = check_url($setting[0]);
  }
  return $options;
}
/**
 * @} End of "module setings".
 */

function infowordpress_get_posts_titles($setid, $blogname, $category, $author, $date) {

  $settings = variable_get('infowordpress_settings', NULL);
  $ws_url = $settings[$setid][0] . (($blogname == '') ? 'xmlrpc.php' : $blogname .'/xmlrpc.php');
  $ws_url = check_url($ws_url);

  $parameters['type'] = 2; //posts recientes

  //set numentry and display
  $parameters['display'] = 2; //get post titles without pager
  $parameters['category'] = $category;
  $parameters['author'] = $author;
  $parameters['date'] = serialize($date);

  $numentry = 0; //get all posts
  $element = '';

  $result = infowordpress_ws_result($ws_url, $numentry, $element, $parameters);
  if (!$result) {
    return FALSE;
  }

  $rows = array();
  foreach ($result['list'] as $unpost) {
    $rows[$unpost['postid']] =  check_plain($unpost['title']);
  }
  unset($result);
  return $rows;
}
function infowordpress_get_wpcateg($setid, $blogname) {

  $settings = variable_get('infowordpress_settings', NULL);
  $ws_url = $settings[$setid][0] . (($blogname == '') ? 'xmlrpc.php' : $blogname .'/xmlrpc.php');
  $ws_url = check_url($ws_url);

  $parameters['type'] = 4; //infocategories

  $numentry = 0; //get all posts
  $element = '';

  $result = infowordpress_ws_result($ws_url, $numentry, $element, $parameters);
  if (!$result) {
    return FALSE;
  }

  $rows = array();
  foreach ($result['list'] as $cat) {
	$rows[$cat['cat_ID']] =  check_plain($cat['cat_name']);
  }
  unset($result);
  return $rows;
}
function infowordpress_get_drupalcateg($iwpid) {
  
  $result = db_query("SELECT cat_ID, cat_name, selected FROM {infowordpress_block_categ} WHERE iwpid=%d ORDER BY cat_name", $iwpid);
  $categories = array();
  $selected = array();
  while ($cat = db_fetch_array($result)) {
    $categories[$cat['cat_ID']] =  check_plain($cat['cat_name']);
	if ($cat['selected']){
	  $selected[] = $cat['cat_ID'];
	}
  }
  return array($categories, $selected);
}
