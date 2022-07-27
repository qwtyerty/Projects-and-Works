using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using Valve.VR.InteractionSystem;

public class MouseLook : MonoBehaviour
{
    public float sensitivity = 500f;
    public Transform player;
    public GameObject ret;
    public Transform destination;

    private Image crosshair;
    private float rot = 0f;
    private GameObject tempLabel;
    private Outline tempoutline;
    private GameObject obj;

    // Start is called before the first frame update
    void Start()
    {
        Cursor.lockState = CursorLockMode.Locked;
        crosshair = ret.GetComponent<Image>();

    }

    // Update is called once per frame
    void Update()
    {
        // Get mouse positions
        float mouseX = Input.GetAxis("Mouse X");
        float mouseY = Input.GetAxis("Mouse Y");

        rot -= mouseY;
        rot = Mathf.Clamp(rot, -90f, 90f);

        //Rotate the camera to account for y movement
        transform.localRotation = Quaternion.Euler(rot, 0f, 0f);

        //Rotate the player for x movement
        player.Rotate(Vector3.up * mouseX);

        //Shoots a particle
        Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
        RaycastHit hit;

        //Collision tests
        if (Physics.Raycast(ray, out hit, Mathf.Infinity))
        {
           //Check if a phsyics object is hit then assign it
            if (hit.collider.gameObject.GetComponent<Interactable>())
            {
                if (obj == null)
                    obj = hit.collider.gameObject;

                //Hide the label if the player is not looking at it
                else if (obj != hit.collider.gameObject)
                {
                    if (obj.gameObject.transform.Find("Label"))
                    {
                        obj.gameObject.transform.Find("Label").gameObject.SetActive(false);
                    }

                    //Disable the outline
                    if (obj.GetComponent<Outline>() == null)
                    {
                        Debug.Log("Object: " + obj.name);
                    }
                    obj.GetComponent<Outline>().enabled = false;

                    //Set tempObject to this now current object.
                    obj = hit.collider.gameObject;

                }

                //Activate the label when the player looks at it
                if (hit.collider.gameObject.transform.Find("Label"))
                {
                    tempLabel = hit.collider.gameObject.transform.Find("Label").gameObject;
                    tempLabel.SetActive(true);
                }

                //Activate the outline when the player looks at it
                if (hit.collider.gameObject.GetComponent<Outline>())
                {
                    tempoutline = hit.collider.gameObject.GetComponent<Outline>();
                    tempoutline.enabled = true;

                }
                crosshair.color = new Color(255, 255, 0);
  
            }

            //Invokes onClick for buttons
            else
            {
                if(hit.collider.gameObject.tag == "Button" && Input.GetMouseButtonDown(0))
                    hit.collider.gameObject.GetComponent<Button>().onClick.Invoke();
                if (tempLabel != null) tempLabel.SetActive(false);
                if (tempoutline != null) tempoutline.enabled = false;
                crosshair.color = new Color(255, 255, 255);
            }
        }

    }
}
